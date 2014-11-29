#if 0
#include <iostream>
#include <gtest/gtest.h>
#include <mockcpp/mockcpp.hpp>
#include <list>
#include <map>
USING_MOCKCPP_NS;
using namespace std;

////////////////////////////////////////////////////////////////////////////////
typedef int MIContext;
    MIContext defaultContext = 0;
typedef int Status;
    const Status S_ERROR = -1;
    const Status S_OK = 0;
    const Status S_CONTINUE = 1;
    const Status S_NOT_IMPLEMENTED = 2;
typedef int MsgId;

struct Msg {
  Msg(MsgId id) : id_(id) {/**/}
  MsgId id() const {return id_;}
private:
  MsgId id_;
};

const MsgId dummyMsg = 0;
const MsgId msg_ack_1 = 1;
const MsgId msg_ack_2 = 2;
const MsgId msg_ack_3 = 3;
const MsgId msg_ack_4 = 4;
const MsgId msg_start = 5;

typedef Status (*MIStep_func) (MIContext& ctxt, const Msg& msg);

struct MIStep {
  MIStep(MIStep_func f) : f_(f) {}
  Status handle_msg(MIContext& ctxt, const Msg& msg) {
    f_(ctxt, msg);
  }
private:
  MIStep_func f_;
};

template<typename F>
MIStep makeMIStep(const F& f) {
  return MIStep(f);
}

Status sync_call_1(MIContext& ctxt, const Msg& msg) {
  std::cout << "sync_call_1" << std::endl;
  return S_OK;
}

Status sync_call_2(MIContext& ctxt, const Msg& msg) {
  std::cout << "sync_call_2" << std::endl;
  return S_OK;
}

Status async_call_1(MIContext& ctxt, const Msg& msg) {
  std::cout << "async_call_1" << std::endl;
  return S_OK;
}

Status on_msg_ack_1(MIContext& ctxt, const Msg& msg) {
  std::cout << "on_msg_ack_1" << std::endl;
  return S_OK;
}

Status on_msg_ack_2(MIContext& ctxt, const Msg& msg) {
  std::cout << "on_msg_ack_2" << std::endl;
  return S_OK;
}

Status on_msg_ack_3(MIContext& ctxt, const Msg& msg) {
  std::cout << "on_msg_ack_3" << std::endl;
  return S_OK;
}

bool foo_function_1(MIContext& ctxt, const Msg& msg) {
  std::cout << "foo_function_1" << std::endl;
  return true;
}

bool bar_function_1(MIContext& ctxt, const Msg& msg) {
  std::cout << "bar_function_1" << std::endl;
  return true;
}

////////////////////////////////////////////////////////////////////////////////
struct MsgAction {
  virtual Status handle_msg(const Msg& msg) = 0;
  virtual Status removeFromMsgMapping() {
    return S_NOT_IMPLEMENTED;
  }
};

struct Dispatcher {
  static Dispatcher& instance() {
    static Dispatcher d;
    return d;
  }
  
  void removeAction(const MsgId msgId) {
    msgMaps_[msgId] = NULL;
  }
  
  void registerAction(const MsgId msgId, MsgAction& action) {
    msgMaps_[msgId] = &action;
  }

  Status handle_msg(const Msg& msg) {
    map<MsgId, MsgAction*>::iterator i = msgMaps_.find(msg.id());
    if (i != msgMaps_.end() && (NULL != i->second)) {
	if (S_CONTINUE != i->second->handle_msg(msg)) {
	  if (S_NOT_IMPLEMENTED == i->second->removeFromMsgMapping()) {
	    msgMaps_[msg.id()] = NULL;
	  }
	}
    }
    return S_OK;
  }
  
  map<MsgId, MsgAction*> msgMaps_;
};


template<MIStep_func f>
struct MIStepHelper : MIStep {
  MIStepHelper() : MIStep(f) {
  }
};

template<typename MI>
struct MsgActionWithContext : MsgAction {
  MsgActionWithContext(MIContext& ctxt, MI& mi): ctxt_(ctxt), mi_(mi) {/**/}
  virtual Status handle_msg(const Msg& msg) {
    return mi_.handle_msg(ctxt_, msg);
  }
  
  private:
    MIContext& ctxt_;
    MI& mi_;
};

template<typename MI>
MsgAction& makeMsgAction(MIContext& ctxt) {
  static MI mi_;
  static MsgActionWithContext<MI> msgAction(ctxt, mi_);
  return msgAction;
}

struct Basic_mi_c {
  Basic_mi_c() { declare_mi_flow(); }
  
  Status handle_msg(MIContext& ctxt, const Msg& msg) {
    std::list<MIStep>::iterator i = sync_calls_.begin();
    for(; i != sync_calls_.end(); ++i) {
      Status ret = i->handle_msg(ctxt, msg);
      
      if (ret != S_OK)
	return ret;
    }
    return S_OK;
  }
  
  void declare_mi_flow() {
    sync_calls_.push_back(makeMIStep(sync_call_1));
    sync_calls_.push_back(makeMIStep(sync_call_2));

    struct on_ack_msgs__of__async_call_1 : MsgAction {
      MIContext& ctxt;
      on_ack_msgs__of__async_call_1(MIContext& ctxt) : ctxt(ctxt) {/**/}
      
      virtual Status handle_msg(const Msg& msg) {
        switch(msg.id()) {
          case msg_ack_1 : { return on_msg_ack_1(ctxt, msg); } break;
          case msg_ack_2 : { return on_msg_ack_2(ctxt, msg); } break;
          case msg_ack_3 : { return on_msg_ack_3(ctxt, msg); } break;
	  case msg_ack_4 : {
	    if (foo_function_1(ctxt, msg) && bar_function_1(ctxt, msg)) {
	      cout << "------------------------" << endl;
	    }
	  } break;
	}
	return S_OK;
      }
      
      virtual Status removeFromMsgMapping() {
	Dispatcher::instance().removeAction(msg_ack_1);
	Dispatcher::instance().removeAction(msg_ack_2);
	Dispatcher::instance().removeAction(msg_ack_3);
	Dispatcher::instance().removeAction(msg_ack_4);
	return S_OK;
      }
    };

    struct async_step_execution {
      static Status async_handling(MIContext& ctxt, const Msg& msg) {
	Status ret =  async_call_1(ctxt, msg);
	if (ret == S_OK) {
	  static on_ack_msgs__of__async_call_1 ackMsgAction(defaultContext);
	  Dispatcher::instance().registerAction(msg_ack_1, ackMsgAction);
	  Dispatcher::instance().registerAction(msg_ack_2, ackMsgAction);
	  Dispatcher::instance().registerAction(msg_ack_3, ackMsgAction);
	  Dispatcher::instance().registerAction(msg_ack_4, ackMsgAction);
	}	
      }
    }; sync_calls_.push_back(makeMIStep(async_step_execution::async_handling));

  }
  
  std::list<MIStep> sync_calls_;
};

////////////////////////////////////////////////////////////////////////////////
struct basic_mi_test : ::testing::Test {
  void TearDown() {
    GlobalMockObject::verify();    
  }
};

TEST_F(basic_mi_test, mi_should_call_trigger_method_of_async_step) {
  MOCKER(sync_call_1).expects(once()).will(returnValue(S_OK)).id("1");
  MOCKER(sync_call_2).expects(once()).after("1").will(returnValue(S_OK)).id("2");
  MOCKER(async_call_1).expects(once()).after("2").will(returnValue(S_OK));

  Dispatcher::instance().registerAction(msg_start, makeMsgAction<Basic_mi_c>(defaultContext));
  
  Dispatcher::instance().handle_msg(msg_start);
}

TEST_F(basic_mi_test, exit_msg_interaction__WHEN_one_step_failed) {
  MOCKER(sync_call_1).stubs().will(returnValue(S_ERROR));
  MOCKER(sync_call_2).expects(never()).will(returnValue(S_OK));

  Dispatcher::instance().registerAction(msg_start, makeMsgAction<Basic_mi_c>(defaultContext));
  
  Dispatcher::instance().handle_msg(msg_start);
}

//------------------------------------------------------------------------------
TEST_F(basic_mi_test, should_call__ack_msg_handling_step__WHEN__got_ack_msg_1) {
  MOCKER(on_msg_ack_1).expects(once()).will(returnValue(S_OK)); 

  Dispatcher::instance().registerAction(msg_start, makeMsgAction<Basic_mi_c>(defaultContext));
  
  Dispatcher::instance().handle_msg(msg_start);
  Dispatcher::instance().handle_msg(msg_ack_1);
}

TEST_F(basic_mi_test, should_call__ack_msg_handling_step__WHEN__got_ack_msg_2) {
  MOCKER(on_msg_ack_2).expects(once()).will(returnValue(S_OK)); 

  Dispatcher::instance().registerAction(msg_start, makeMsgAction<Basic_mi_c>(defaultContext));
  
  Dispatcher::instance().handle_msg(msg_start);
  Dispatcher::instance().handle_msg(msg_ack_2);
}

TEST_F(basic_mi_test, should_call__ack_msg_handling_step__WHEN__got_ack_msg_3) {
  MOCKER(on_msg_ack_3).expects(once()).will(returnValue(S_OK)); 

  Dispatcher::instance().registerAction(msg_start, makeMsgAction<Basic_mi_c>(defaultContext));
  
  Dispatcher::instance().handle_msg(msg_start);
  Dispatcher::instance().handle_msg(msg_ack_3);
}

//------------------------------------------------------------------------------
TEST_F(basic_mi_test, should_keep_in_Dispatcher__if_steps_returns_S_CONTINUE) {
  MOCKER(on_msg_ack_1).expects(exactly(2)).will(returnValue(S_CONTINUE)); 

  Dispatcher::instance().registerAction(msg_start, makeMsgAction<Basic_mi_c>(defaultContext));
  
  Dispatcher::instance().handle_msg(msg_start);
  Dispatcher::instance().handle_msg(msg_ack_1);
  Dispatcher::instance().handle_msg(msg_ack_1);
}

TEST_F(basic_mi_test, should_remove_from_Dispatcher__if_steps_returns_nother_than__S_CONTINUE) {
  MOCKER(on_msg_ack_1).expects(once()).will(returnValue(S_OK)); 

  Dispatcher::instance().registerAction(msg_start, makeMsgAction<Basic_mi_c>(defaultContext));
  
  Dispatcher::instance().handle_msg(msg_start);
  Dispatcher::instance().handle_msg(msg_ack_1);
  Dispatcher::instance().handle_msg(msg_ack_1);
}

TEST_F(basic_mi_test,
should_remove_all_ack_handlers_in_same_group_from_Dispatcher__if_NOT_return_S_CONTINUE) {
  
  MOCKER(on_msg_ack_1).expects(once()).will(returnValue(S_OK)); 
  MOCKER(on_msg_ack_2).expects(never()).will(returnValue(S_OK)); 
  MOCKER(on_msg_ack_3).expects(never()).will(returnValue(S_OK)); 

  Dispatcher::instance().registerAction(msg_start, makeMsgAction<Basic_mi_c>(defaultContext));
  
  Dispatcher::instance().handle_msg(msg_start);
  Dispatcher::instance().handle_msg(msg_ack_1);
  
  Dispatcher::instance().handle_msg(msg_ack_1);
  Dispatcher::instance().handle_msg(msg_ack_2);
  Dispatcher::instance().handle_msg(msg_ack_3);
}

//------------------------------------------------------------------------------
TEST_F(basic_mi_test, DISABLED_support_guard_task) {
}

TEST_F(basic_mi_test, xxxxxx) {
  MOCKER(foo_function_1).expects(exactly(1)).will(returnValue(true)); 

  Dispatcher::instance().registerAction(msg_start, makeMsgAction<Basic_mi_c>(defaultContext));
  
  Dispatcher::instance().handle_msg(msg_start);
  Dispatcher::instance().handle_msg(msg_ack_4);
}

#endif
