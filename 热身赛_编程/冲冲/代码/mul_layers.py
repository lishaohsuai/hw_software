# -*- coding: utf-8 -*-
import math
import datetime
import sys
import numpy as np

class LR:
    def __init__(self, train_file_name, test_file_name, predict_result_file_name):
        self.train_file = train_file_name
        self.predict_file = test_file_name
        self.predict_result_file = predict_result_file_name
        self.predict_label_file="data/answer.txt" 
        self.max_iters = 50   #最大迭代次数
        self.rate = 0.45      #学习率
        self.batch_size=8000       #训练批次大小，经测试，批次越大，效果越好。
        self.keep_prob=1        #输出层随机失活率
        self.layers_dims=[64,1]  #隐藏层神经元个数
        self.decay_step=20    #迭代多少次，学习率更新一次
        self.decay_rate=0.96  #学习率更新次数
        self.Norm_flag=False  #输入数据是否标准化处理
        self.feats,self.labels =self.loadTrainData()  #加载训练数据
        self.feats_test,self.labels_test=self.loadTestData()   #加载测试数据
        self.parameters=self.init_parameters()   #初始化参数W,B


    def loadDataSet(self,file_name, label_existed_flag):
        feats = []
        labels = []
        fr = open(file_name)
        lines = fr.readlines()
        for line in lines:
            temp = []
            allInfo = line.strip().split(',')
            dims = len(allInfo)
            if label_existed_flag == 1:
                for index in range(dims-1):
                    temp.append(float(allInfo[index]))
                feats.append(temp)
                labels.append(float(allInfo[dims-1]))
            else:
                for index in range(dims):
                    temp.append(float(allInfo[index]))
                feats.append(temp)
        
        if label_existed_flag == 0:    #加载测试集结果，线上跑不能读取测试集结果
            f_a = open(self.predict_label_file, 'r')
            lines = f_a.readlines()
            for line in lines:
                labels.append(int(float(line.strip())))
            f_a.close()
        
        fr.close()

        if self.Norm_flag :   #求均值，标准差，进行标准化处理
            mu = np.mean(feats, axis=1,keepdims=True)
            sigma = np.std(feats, axis=1,keepdims=True)
            feats= (feats-mu)/sigma
        else:    #对输入数据不进行标准化处理，只进行归一化处理
            feats = np.array(feats)*2-1
        
        labels = np.array(labels).reshape(1, -1)  
        return feats.T, labels   #数组大小[C,N],[1,N]  C为特征维度，N为样本总数

    def loadTrainData(self):
        feats, labels = self.loadDataSet(self.train_file, 1) #[C,N]
        C,N=feats.shape
        self.layers_dims.insert(0,C)  
        # 对输入数据进行分批次处理。
        feats=feats[:,0:N-N%self.batch_size].reshape(-1,C,self.batch_size)
        labels=labels[:,0:N-N%self.batch_size].reshape(-1,1,self.batch_size)
        return feats,labels  #数组大小 [m,C,B],[m,1,B]   m为总的批次个数，B为批次大小


    def loadTestData(self):
        feats, labels = self.loadDataSet(self.predict_file, 0)
        return feats,labels #[m,C,B],[m,1,B]

    def init_parameters(self):
        parameters={}
        L=len(self.layers_dims)  #神经网络层数
        # 初始化参数W,B
        for l in range(1,L):  
            parameters["W"+str(l)]=np.random.randn(self.layers_dims[l],self.layers_dims[l-1])*np.sqrt(2/self.layers_dims[l - 1])
            parameters["b"+str(l)]=np.zeros((self.layers_dims[l],1))
        return parameters

    
    #向前传播
    def forward(self,X,train_flag=1):  
        # 线性激活函数
        def linear_activation(A_pre,l,code,R=None):
            W=self.parameters["W"+str(l)]
            b=self.parameters["b"+str(l)]
            # 防止过拟合，输出层进行随机失活
            if R is None:
                Z=np.dot(W,A_pre)+b
            else:
                A_pre *= R
                A_pre /=self.keep_prob
                Z=np.dot(W,A_pre)+b
            # 激活函数
            if code =='relu':
                A = np.maximum(0,Z)
            elif code =='sigmoid':
                A=1/(1+np.exp(-Z))
            cache={'A_pre':A_pre,'W':W,'b':b,'Z':Z,'A':A} 
            return A,cache  #返回该层的输出和参数缓存
        
        A=X 
        caches=[]
        L=len(self.parameters)//2  
        for l in range(1,L): 
            A_pre=A
            A,cache = linear_activation(A_pre,l,'relu')
            caches.append(cache)
        # last layer 
        R=0
        if train_flag ==1:
            # dropout矩阵 
            R=np.random.rand(A.shape[0],A.shape[1]) < self.keep_prob 
            AL,cache=linear_activation(A,L,'sigmoid',R)
        else:
            AL,cache=linear_activation(A,L,'sigmoid')
        caches.append(cache)
        return R,AL,caches  #返回dropout矩阵，输出，参数
    #向后传播，梯度更新
    def backward(self,caches,R,AL,y):  

        def sigmoid_backward(A,y):
            dZ=A-y
            return dZ

        def relu_backward(dA,Z):
            dZ = np.array(dA, copy=True)
            dZ[Z <= 0] = 0
            return dZ
        
        def linear_backward(dZ,A_pre,W):
            m=dZ.shape[1]
            dW=np.dot(dZ,A_pre.T)/m
            db=np.sum(dZ,axis=1,keepdims=True)/m
            dA_pre=np.dot(W.T,dZ)
            return dA_pre,dW,db
        
        def update_parameters(dW,db,L):
            self.parameters['W'+str(L)]=self.parameters['W'+str(L)]-self.rate*dW
            self.parameters['b'+str(L)]=self.parameters['b'+str(L)]-self.rate*db
        #last layers，即输出层的梯度下降
        L=len(self.parameters)//2 
        dZ=sigmoid_backward(AL,y)
        dA_pre,dW,db=linear_backward(dZ,caches[L-1]['A_pre'],self.parameters['W'+str(L)])
        dA_pre *=R
        dA_pre /=self.keep_prob
        update_parameters(dW,db,L)
        # 隐藏层的梯度下降
        for l in reversed(range(1,L)):
            dA=dA_pre
            dZ=relu_backward(dA,caches[l-1]['Z'])
            dA_pre,dW,db=linear_backward(dZ,caches[l-1]['A_pre'],self.parameters['W'+str(l)])
            update_parameters(dW,db,l)

    def savePredictResult(self,labels_predict):
        #保存预测结果
        f = open(self.predict_result_file, 'w')
        for i in range(len(labels_predict)):
            f.write(str(labels_predict[i])+"\n")
        f.close()

    def model(self):
        def cost_accuracy(Y_pre,Y):
            # cost
            logprobs = np.multiply(np.log(Y_pre), Y) + np.multiply((1 - Y), np.log(1 - Y_pre))
            cost = np.squeeze(- np.sum(logprobs))
            # accuracy
            Y_pre=np.round(Y_pre)
            accuracy_sum=self.batch_size - np.sum(np.abs(Y_pre - Y))

            return cost,accuracy_sum
        self.init_parameters()
        costs=[]
        steps=0
        #开始迭代
        for i in range(self.max_iters):
            N=self.feats.shape[0]
            cost_sum=0
            accuracy_sum=0
            for k in range(N):  # the index of batch
                X=self.feats[k]
                Y=self.labels[k]

                R,AL,caches=self.forward(X)
                self.backward(caches,R,AL,Y)

                # cost,accuracy
                cost,accuracy=cost_accuracy(AL,Y)
                cost_sum += cost
                accuracy_sum += accuracy

                steps +=1  # iterations
                # 更新学习率
                if steps % self.decay_step ==0:
                    self.rate*= self.decay_rate
            
            train_accuracy=accuracy_sum/(self.batch_size*N)
            train_cost=cost/(self.batch_size*N)

            # -------test-------
            R,test_pre,_=self.forward(self.feats_test,0)
            test_pre=np.round(test_pre)
            test_accuracy=1-np.sum(np.abs(test_pre - self.labels_test))/self.labels_test.shape[1]

            print ('Epoch %d  train_accuracy：%.2f  train_cost: %f  test_accuracy：%.2f' 
                    %(i+1,train_accuracy,train_cost,test_accuracy))

def print_help_and_exit():
    print("usage:python3 main.py train_data.txt test_data.txt predict.txt [debug]")
    sys.exit(-1)

def parse_args():
    debug = False
    if len(sys.argv) == 2:
        if sys.argv[1] == 'debug':
            print("test mode")
            debug = True
        else:
            print_help_and_exit()
    return debug

if __name__ == "__main__":
    start_Time = datetime.datetime.now()
    debug = parse_args()
    train_file =  "data/train_data.txt"
    test_file = "data/test_data.txt"
    predict_file = "data/result.txt" 
    lr = LR(train_file, test_file, predict_file)
    lr.model()
    # 输出训练时间（秒）
    print((datetime.datetime.now()-start_Time).total_seconds()*2)
    
    if debug:
        answer_file =lr.predict_label_file
        f_a = open(answer_file, 'r')
        f_p = open(predict_file, 'r')
        a = []
        p = []
        lines = f_a.readlines()
        for line in lines:
            a.append(int(float(line.strip())))
        f_a.close()

        lines = f_p.readlines()
        for line in lines:
            p.append(int(float(line.strip())))
        f_p.close()

        print("answer lines:%d" % (len(a)))
        print("predict lines:%d" % (len(p)))

        errline = 0
        for i in range(len(a)):
            if a[i] != p[i]:
                errline += 1

        accuracy = (len(a)-errline)/len(a)
        print("accuracy:%f" %(accuracy))