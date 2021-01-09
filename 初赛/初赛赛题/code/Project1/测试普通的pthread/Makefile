LOCAL_LIBRARY +=  -L./lib 
LOCAL_LDFLAGS += -lm -lpthread -ldl
LOCAL_CFLAGS += -I./include

CC:=g++

TARGETS1 = lesson_02
SRCS1 = test.cpp
OBJS1 = $(patsubst %.cpp, %.o, $(SRCS1))

CFLAGS += $(LOCAL_CFLAGS)
LDFLAGS += $(LOCAL_LIBRARY) $(LOCAL_LDFLAGS)

$(info $(OBJS))
$(info $(TARGETS))

all: $(TARGETS1)


$(TARGETS1):$(OBJS1)
	$(CC)  -o $@ $^  $(LDFLAGS) $(CFLAGS)

$(OBJS1): %.o:%.cpp
	$(CC) -c $< -o $@ $(CFLAGS)

clean :
	@rm -rf $(TARGETS1) $(OBJS1)

#.SUFFIXES:
.PHONY : all clean
