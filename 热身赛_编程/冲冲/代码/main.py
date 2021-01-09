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
        self.max_iters = 50
        self.rate = 0.45
        self.batch_size=8000
        self.decay_step=20
        self.decay_rate=0.96
        self.keep_prob=1
        self.Norm_flag=True
        self.feats,self.labels =self.loadTrainData()
        self.feats_test,self.labels_test=self.loadTestData()
        self.parameters=self.init_parameters()


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
        
        fr.close()
        #print("最大值：%f 最小值：%f 平均值：%f" %(np.max(feats),np.min(feats),np.mean(feats)))
        # 标准化
        if self.Norm_flag :
            mu = np.mean(feats, axis=1,keepdims=True)
            sigma = np.std(feats, axis=1,keepdims=True)
            feats= (feats-mu)/sigma
        else:
            feats = np.array(feats)*2-1
        
        labels = np.array(labels).reshape(1, -1)
        return feats.T, labels

    def loadTrainData(self):
        feats, labels = self.loadDataSet(self.train_file, 1) #[C,N]
        C,N=feats.shape
        feats=feats[:,0:N-N%self.batch_size].reshape(-1,C,self.batch_size)
        labels=labels[:,0:N-N%self.batch_size].reshape(-1,1,self.batch_size)
        return feats,labels  #[:,C,B],[:,1,B]


    def loadTestData(self):
        feats, labels = self.loadDataSet(self.predict_file, 0)
        return feats,labels 

    def init_parameters(self):
        parameters={}
        parameters["W"]=np.random.randn(1,self.feats.shape[1])*np.sqrt(2/self.feats.shape[1])
        parameters["b"]=np.zeros((1,1))
        return parameters

    def forward(self,X,train_flag=1,dropout_flag=0):
        W=self.parameters["W"]
        b=self.parameters["b"]
        if train_flag ==1 and dropout_flag==1 :
            R=np.random.rand(A.shape[0],A.shape[1]) < self.keep_prob
            X *= R
            X /=self.keep_prob
            Z=np.dot(W,X)+b
        else:
            Z=np.dot(W,X)+b
            
            # sigmoid
        A=1/(1+np.exp(-Z))
        cache={'X':X,'W':W,'b':b,'Z':Z,'A':A} 
        return A,cache

    def backward(self,cache,y):
        X=cache["X"]
        A=cache["A"]
        W=self.parameters['W']
        b=self.parameters['b']

        dZ=A-y
        m=dZ.shape[1]
        dW=np.dot(dZ,X.T)/m
        db=np.sum(dZ,axis=1,keepdims=True)/m
        self.parameters['W'] -=self.rate*dW
        self.parameters['b'] -=self.rate*db

    def savePredictResult(self,labels_predict):
        
        f = open(self.predict_result_file, 'w')
        for i in range(len(labels_predict)):
            f.write(str(labels_predict[i])+"\n")
        f.close()

    def model(self):
        def cost_accuracy(Y_pre,Y):
            # cost
            #logprobs = np.multiply(np.log(Y_pre), Y) + np.multiply((1 - Y), np.log(1 - Y_pre))
            #cost = np.squeeze(- np.sum(logprobs))
            cost=0
            # accuracy
            Y_pre=np.round(Y_pre)
            accuracy_sum=self.batch_size - np.sum(np.abs(Y_pre - Y))
            return cost,accuracy_sum  #/self.batch_size
        
        def test_predict():
            # -------test-------
            test_pre,_=self.forward(self.feats_test,0)
            test_pre=np.round(test_pre).astype(np.int)
            self.savePredictResult(np.squeeze(test_pre))

        steps=0
        for i in range(self.max_iters):
            N=self.feats.shape[0]
            accuracy_sum=0
            cost_sum=0
            for k in range(N):  # the index of batch
                X=self.feats[k]
                Y=self.labels[k]

                A,caches=self.forward(X)
                self.backward(caches,Y)
                cost,accuracy=cost_accuracy(A,Y)
                accuracy_sum += accuracy
                cost_sum +=cost
                #if accuracy_sum >70:
                    #break
                steps +=1  # iterations
                if steps % self.decay_step ==0:
                    self.rate*= self.decay_rate
            
            #print("EPOCH %d train_accuracy: %.2f train_cost: %f" %(i+1,accuracy_sum/(N*self.batch_size),cost_sum/(N*self.batch_size)))
        test_predict()

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
    train_file =  "/data/train_data.txt"
    test_file = "/data/test_data.txt"
    predict_file = "/projects/student/result.txt" #/projects/student
    lr = LR(train_file, test_file, predict_file)
    lr.model()
    print((datetime.datetime.now()-start_Time).total_seconds()*2)
    
    if debug:
        answer_file = "/projects/student/answer.txt" 
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
