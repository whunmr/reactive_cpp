#include <iostream>
#include <gtest/gtest.h>
#include <mockcpp/mockcpp.hpp>
#include <list>
#include <set>
#include <functional>
#include <map>
USING_MOCKCPP_NS;
using namespace std;

////////////////////////////////////////////////////////////////////////////////
#define _line_var_3(line, ...) __VA_ARGS__##_##line
#define _line_var_2(line, ...)   _line_var_3(line, __VA_ARGS__)
#define _line_var_1(LINE__, ...) _line_var_2(__##LINE__, __VA_ARGS__)
#define line_var(...)            _line_var_1(LINE__, __VA_ARGS__)
////////////////////////////////////////////////////////////////////////////////
typedef int MIContext;
    MIContext defaultContext = 0;  //TODO:
typedef int Status;
    const Status S_FAILED = -2;
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
const MsgId nested_msg_ack_1 = 101;
const MsgId nested_msg_ack_2 = 102;
const MsgId msg_ack_2 = 2;
const MsgId msg_ack_3 = 3;
const MsgId msg_ack_4 = 4;
const MsgId msg_start = 5;
const MsgId msg_timeout = 6;
const MsgId msg_succeeded = 7;
const MsgId msg_failed = 8;
const MsgId msg_tick = 9;

const int _2s = 2;

struct MIActor {
  MIActor& operator--() { return *this; }
  MIActor& operator--(int) { return *this; }
  template<typename T> T& operator>(T& t) { return t;}
  template<typename T> T& operator<(T& t) { return t;}
  template<typename T> T& operator-(T& t) { return t;}
} self, __remote_actor, __dummyActor, sysd;
////////////////////////////////////////////////////////////////////////////////
typedef Status (*Step_func) (MIContext& ctxt, const Msg& msg);

struct Step {
  virtual Status handle_msg(MIContext& ctxt, const Msg& msg) = 0;
};

struct MIFuncStep : Step {
  MIFuncStep(Step_func f) : f_(f) {}
  
  virtual Status handle_msg(MIContext& ctxt, const Msg& msg) {
    f_(ctxt, msg);
  }
  
private:
  Step_func f_;
};

template<Step_func step_func>
Step* makeStep() {
  static MIFuncStep step(step_func);
  return &step;
}

#define def_sync_call(method_name) \
Status method_name(MIContext& ctxt, const Msg& msg) { \
  std::cout << #method_name << std::endl;\
  return S_OK;\
}

def_sync_call(sync_call_0);
def_sync_call(sync_call_1);
def_sync_call(sync_call_2);
def_sync_call(sync_call_3);
def_sync_call(sync_call_4);
def_sync_call(sync_call_5);
def_sync_call(sync_call_6);

def_sync_call(async_call_0);
def_sync_call(async_call_1);
def_sync_call(async_call_2);
def_sync_call(async_call_3);
def_sync_call(async_call_4);
def_sync_call(async_call_5);
def_sync_call(async_call_6);

def_sync_call(nested_async_call_1);
def_sync_call(nested_async_call_2);
def_sync_call(nested_async_call_3);
def_sync_call(nested_async_call_4);
def_sync_call(nested_async_call_5);
def_sync_call(nested_async_call_6);

def_sync_call(on_nested_msg_ack_1);
def_sync_call(on_nested_msg_ack_2);
def_sync_call(on_nested_msg_ack_3);
def_sync_call(on_nested_msg_ack_4);
def_sync_call(on_nested_msg_ack_5);
def_sync_call(on_nested_msg_ack_6);

def_sync_call(on_msg_ack_1);
def_sync_call(on_msg_ack_2);
def_sync_call(on_msg_ack_3);
def_sync_call(on_msg_ack_4);
def_sync_call(on_msg_ack_5);
def_sync_call(on_msg_ack_6);

def_sync_call(on_guard_task);

def_sync_call(sync_call_nested_5);


bool foo_function_1(MIContext& ctxt, const Msg& msg) {
  std::cout << "foo_function_1" << std::endl;
  return true;
}

bool bar_function_1(MIContext& ctxt, const Msg& msg) {
  std::cout << "bar_function_1" << std::endl;
  return true;
}

Status sync_call_nested_1(MIContext& ctxt, const Msg& msg) {
  if (foo_function_1(defaultContext, msg)
      && bar_function_1(defaultContext, msg)) {
    return S_OK;
  }
  return S_FAILED;
}

Status sync_call_nested_6(MIContext& ctxt, const Msg& msg) {
  std::cout << "sync_call_nested_6" << std::endl;
  return S_OK;
}

////////////////////////////////////////////////////////////////////////////////
struct MsgAction {
  virtual Status handle_msg(const Msg& msg) = 0;
};

struct Dispatcher {
  static Dispatcher& inst() {
    static Dispatcher d;
    return d;
  }
  
  void removeAction(const MsgId msgId) {
    //cout << "remove action for msg: " << msgId << endl;
    //msgMaps_[msgId] = NULL;
  }
  
  void registerAction(const MsgId msgId, MsgAction& action) {
    cout << "register action for msg: " << msgId << endl;
    msgMaps_[msgId] = &action;
  }

  Status handle_msg(const Msg& msg) {
    map<MsgId, MsgAction*>::iterator i = msgMaps_.find(msg.id());
    if (i != msgMaps_.end() && (NULL != i->second)) {
	return i->second->handle_msg(msg);
    } else {
      cout << "unknown msg:" << msg.id() << endl;
    }
    return S_OK;
  }
  
  map<MsgId, MsgAction*> msgMaps_;
};

typedef Status (*MsgHandler)(const Msg& msg);

template<MsgHandler remove_ack_msgs_from_dispatcher>
struct AckMsgAction : MsgAction {
  virtual Status handle_msg(const Msg& msg) {
    Status ret = do_handle_msg(msg);
    if (ret != S_CONTINUE) {
      remove_ack_msgs_from_dispatcher(msg);
    }
    return ret;
  }
  
private:
  virtual Status do_handle_msg(const Msg& msg) = 0;
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

template<MsgId id>
struct MsgIdWrapper {
  static const MsgId msg_id;
};

template<MsgId id>
const MsgId MsgIdWrapper<id>::msg_id = id;

MIContext specialCtxt = 99;

template<typename MI>
struct MIData {
  static std::set<MsgId> ack_msgs_;
};

template<typename MI>
std::set<MsgId> MIData<MI>::ack_msgs_;



////////////////////////////////////////////////////////////////////////////////
struct BaseMI {
  Status handle_msg(MIContext& ctxt, const Msg& msg) {
    std::list<Step*>::iterator i = sync_calls_.begin();
    for(; i != sync_calls_.end(); ++i) {
      if (*i != NULL) {
	Status ret = (*i)->handle_msg(ctxt, msg);
	if (ret != S_OK)
	  return ret;	
      }
    }
    return S_OK;
  }
  
protected:
  std::list<Step*> sync_calls_;
};


#define _step(func) sync_calls_.push_back(makeStep<func>());

#define MI_ACK_MSGS MIData<current_mi_step>::ack_msgs_ //clear somewhere

#define msg_ack_handler_name(msg)     msg##_##_handler
#define msg_ack_handler_inst(msg) msg##_##_handler_##_instance

#define on_msg_ack(MSG, MSG_HANDLER)                                               \
  static struct msg_ack_handler_name(MSG) : MsgIdWrapper<MSG>		           \
				          , AckMsgAction<on_receive_ack_msg> {	   \
    virtual Status do_handle_msg(const Msg& msg) {				   \
      return MSG_HANDLER(defaultContext, msg);					   \
    }										   \
  } msg_ack_handler_inst(MSG);						           \
  Dispatcher::inst().registerAction( msg_ack_handler_inst(MSG).msg_id	           \
				   , msg_ack_handler_inst(MSG));		   \
  MIData<current_mi_step>::ack_msgs_.insert(msg_ack_handler_inst(MSG).msg_id); 


#define def_mi(mi_name)                         \
struct mi_name : BaseMI {			\
  mi_name() { mi_declare_steps_flow(); }	\
  typedef mi_name current_mi_step;		\
  void mi_declare_steps_flow() 

#define ____async_step(async_step_trigger_func)                                                \
    static struct async_step_trigger_func##_handler : Step /*add guardTask.*/{               \
      static Status on_receive_ack_msg(const Msg& msg) {				       \
	cout << "---xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx----->async_step_trigger_func" << endl; return S_OK;   \
      }											       \
											       \
      virtual Status handle_msg(MIContext& ctxt, const Msg& msg) {			       \
	Status ret = async_step_trigger_func(ctxt, msg);				       \
	if (ret == S_OK) {handle_msg_helper(ctxt, msg);}                                       \
	return ret;              			                                       \
      }											       \
      virtual void handle_msg_helper(MIContext& ctxt, const Msg& msg) 

#define async_step_end \
} line_var(async_step_handler); sync_calls_.push_back(&line_var(async_step_handler));

  
#define ____on_msg_ack_ex(msg_id)                                                              \
	static struct msg_id##_handler : MsgIdWrapper<msg_id>                                  \
				       , AckMsgAction<on_receive_ack_msg> {		       \
	  typedef MsgIdWrapper<msg_id> current_mi_step;					       \
											       \
          virtual Status do_handle_msg(const Msg& msg) {				       \
	    struct MI_for_##msg_id : BaseMI {						       \
              MI_for_##msg_id() { mi_declare_steps_flow(); }				       \
	      void mi_declare_steps_flow() 

#define on_msg_ack_ex_end  }                                                                   \
  	    line_var(on_msg_ack_ex_inner_mi); return line_var(on_msg_ack_ex_inner_mi).handle_msg(defaultContext, msg); \
	  }										       \
	} line_var(on_msg_ack_ex_handler_inst);	Dispatcher::inst().registerAction(line_var(on_msg_ack_ex_handler_inst).msg_id, line_var(on_msg_ack_ex_handler_inst)); MI_ACK_MSGS.insert(line_var(on_msg_ack_ex_handler_inst).msg_id);

#define def_mi_end };
		
#define async_step(...) __dummyActor; ____async_step(__VA_ARGS__)
#define ack(...) __dummyActor; on_msg_ack(__VA_ARGS__)
#define ack_ex(...) __dummyActor; ____on_msg_ack_ex(__VA_ARGS__)

def_mi(Basic_mi_c) {
                         _step(sync_call_1);
                         _step(sync_call_2);
    sysd<----------------async_step(async_call_1) {
        sysd------------>ack(msg_ack_1, on_msg_ack_1);
        sysd------------>ack(msg_ack_2, on_msg_ack_2);
        sysd------------>ack(msg_ack_3, on_msg_ack_3);
        
	sysd------------>ack_ex(msg_ack_4) {
	                 _step(sync_call_nested_1);
	                 _step(sync_call_nested_5);
	                 _step(sync_call_nested_6);
	    sysd<--------async_step(nested_async_call_1) {
	        self<----ack(nested_msg_ack_1, on_nested_msg_ack_1);
	        self<----ack(nested_msg_ack_2, on_nested_msg_ack_2);
	    } async_step_end;
	} on_msg_ack_ex_end;
    } async_step_end;
} def_mi_end;

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

  Dispatcher::inst().registerAction(msg_start, makeMsgAction<Basic_mi_c>(defaultContext));
  
  Dispatcher::inst().handle_msg(msg_start);
}

TEST_F(basic_mi_test, exit_msg_interaction__WHEN_one_step_failed) {
  MOCKER(sync_call_1).stubs().will(returnValue(S_ERROR));
  MOCKER(sync_call_2).expects(never()).will(returnValue(S_OK));

  Dispatcher::inst().registerAction(msg_start, makeMsgAction<Basic_mi_c>(defaultContext));
  
  Dispatcher::inst().handle_msg(msg_start);
}

//------------------------------------------------------------------------------
TEST_F(basic_mi_test, should_call__ack_msg_handling_step__WHEN__got_ack_msg_1) {
  MOCKER(on_msg_ack_1).expects(once()).will(returnValue(S_OK)); 

  Dispatcher::inst().registerAction(msg_start, makeMsgAction<Basic_mi_c>(defaultContext));
  
  Dispatcher::inst().handle_msg(msg_start);
  Dispatcher::inst().handle_msg(msg_ack_1);
}

TEST_F(basic_mi_test, should_call__ack_msg_handling_step__WHEN__got_ack_msg_2) {
  MOCKER(on_msg_ack_2).expects(once()).will(returnValue(S_OK)); 

  Dispatcher::inst().registerAction(msg_start, makeMsgAction<Basic_mi_c>(defaultContext));
  
  Dispatcher::inst().handle_msg(msg_start);
  Dispatcher::inst().handle_msg(msg_ack_2);
}

TEST_F(basic_mi_test, should_call__ack_msg_handling_step__WHEN__got_ack_msg_3) {
  MOCKER(on_msg_ack_3).expects(once()).will(returnValue(S_OK)); 

  Dispatcher::inst().registerAction(msg_start, makeMsgAction<Basic_mi_c>(defaultContext));
  
  Dispatcher::inst().handle_msg(msg_start);
  Dispatcher::inst().handle_msg(msg_ack_3);
}

//------------------------------------------------------------------------------
TEST_F(basic_mi_test, should_keep_in_Dispatcher__if_steps_returns_S_CONTINUE) {
  MOCKER(on_msg_ack_1).expects(exactly(2)).will(returnValue(S_CONTINUE)); 

  Dispatcher::inst().registerAction(msg_start, makeMsgAction<Basic_mi_c>(defaultContext));
  
  Dispatcher::inst().handle_msg(msg_start);
  Dispatcher::inst().handle_msg(msg_ack_1);
  Dispatcher::inst().handle_msg(msg_ack_1);
}

TEST_F(basic_mi_test, DISABLED_should_remove_from_Dispatcher__if_steps_returns_nother_than__S_CONTINUE) {
  MOCKER(on_msg_ack_1).expects(once()).will(returnValue(S_OK)); 

  Dispatcher::inst().registerAction(msg_start, makeMsgAction<Basic_mi_c>(defaultContext));
  
  Dispatcher::inst().handle_msg(msg_start);
  Dispatcher::inst().handle_msg(msg_ack_1);
  Dispatcher::inst().handle_msg(msg_ack_1);
}

TEST_F(basic_mi_test,
DISABLED_should_remove_all_ack_handlers_in_same_group_from_Dispatcher__if_NOT_return_S_CONTINUE) {
  
  MOCKER(on_msg_ack_1).expects(once()).will(returnValue(S_OK)); 
  MOCKER(on_msg_ack_2).expects(never()).will(returnValue(S_OK)); 
  MOCKER(on_msg_ack_3).expects(never()).will(returnValue(S_OK)); 

  Dispatcher::inst().registerAction(msg_start, makeMsgAction<Basic_mi_c>(defaultContext));
  
  Dispatcher::inst().handle_msg(msg_start);
  Dispatcher::inst().handle_msg(msg_ack_1);
  
  Dispatcher::inst().handle_msg(msg_ack_1);
  Dispatcher::inst().handle_msg(msg_ack_2);
  Dispatcher::inst().handle_msg(msg_ack_3);
}

TEST_F(basic_mi_test, support_write_complex_expression_in_ack_msg_handler_like_for_msg_ack_4) {
  MOCKER(foo_function_1).expects(exactly(1)).will(returnValue(true)); 

  Dispatcher::inst().registerAction(msg_start, makeMsgAction<Basic_mi_c>(defaultContext));
  
  Dispatcher::inst().handle_msg(msg_start);
  Dispatcher::inst().handle_msg(msg_ack_4);
}

//------------------------------------------------------------------------------
TEST_F(basic_mi_test, should_able_to_call_nested_message_interaction) {
  MOCKER(sync_call_nested_5).expects(once()).will(returnValue(S_OK));
  MOCKER(sync_call_nested_6).expects(once()).will(returnValue(S_OK));
  
  Dispatcher::inst().registerAction(msg_start, makeMsgAction<Basic_mi_c>(defaultContext));
  
  Dispatcher::inst().handle_msg(msg_start);
  Dispatcher::inst().handle_msg(msg_ack_4);
}

TEST_F(basic_mi_test, should_able_to_call_nested_message__and_nested_ack_msg_handler) {
  MOCKER(sync_call_nested_5).expects(once()).will(returnValue(S_OK));
  MOCKER(sync_call_nested_6).expects(once()).will(returnValue(S_OK));
  MOCKER(nested_async_call_1).expects(once()).will(returnValue(S_OK));
  MOCKER(on_nested_msg_ack_1).expects(once()).will(returnValue(S_OK));

  Dispatcher::inst().registerAction(msg_start, makeMsgAction<Basic_mi_c>(defaultContext));
  
  Dispatcher::inst().handle_msg(msg_start);
  Dispatcher::inst().handle_msg(msg_ack_4);
  Dispatcher::inst().handle_msg(nested_msg_ack_1);
}
  
//------------------------------------------------------------------------------
def_mi(Basic_mi_x) {
                                 _step(sync_call_1);
                                 _step(sync_call_2);
        sysd<--------------------async_step(async_call_1) {
        	sysd------------>ack(msg_ack_1, on_msg_ack_1);
        	sysd------------>ack(msg_ack_2, on_msg_ack_2);
                
        	sysd------------>ack_ex(msg_ack_4)
                {
                                 _step(sync_call_nested_1);
        		sysd<----async_step(nested_async_call_1)
                        {
        		         sysd---->ack(nested_msg_ack_1, on_nested_msg_ack_1);
        		         sysd---->ack(nested_msg_ack_2, on_nested_msg_ack_2);
        		         sysd---->ack(msg_ack_1, on_nested_msg_ack_3);		    
        		} async_step_end;
        	} on_msg_ack_ex_end;
        }async_step_end;
} def_mi_end;

TEST_F(basic_mi_test, should_correct_call_handler_for_same_msg__msg_ack_1) {
  MOCKER(on_nested_msg_ack_3).expects(once()).will(returnValue(S_OK));

  Dispatcher::inst().registerAction(msg_start, makeMsgAction<Basic_mi_x>(defaultContext));
  Dispatcher::inst().handle_msg(msg_start);
  Dispatcher::inst().handle_msg(msg_ack_4);
  Dispatcher::inst().handle_msg(msg_ack_1);
}

//------------------------------------------------------------------------------
def_mi(Basic_mi_y) {
    _step(sync_call_1);
    _step(sync_call_2);
    sysd<------------------------async_step(async_call_1)
    {
	sysd-------------------->ack(msg_ack_1, on_msg_ack_1);
	sysd-------------------->ack(msg_ack_2, on_msg_ack_2);
	sysd-------------------->ack_ex(msg_ack_4)
        {
             _step(sync_call_nested_1);
             sysd<----------------async_step(nested_async_call_1)
             {
                 sysd------------>ack_ex(nested_msg_ack_1)
                 {
                                   _step(on_nested_msg_ack_1);
                      sysd<--------async_step(nested_async_call_1) {
			  sysd---->ack(nested_msg_ack_2, on_nested_msg_ack_2);
                      }                                               async_step_end;
                 }                                                    on_msg_ack_ex_end;
            }                                                         async_step_end;
	}                                                             on_msg_ack_ex_end;
    }                                                                 async_step_end;
} def_mi_end;


TEST_F(basic_mi_test, should_correct_call_handler_for_nested_ack_ex) {
  MOCKER(on_nested_msg_ack_1).expects(once()).will(returnValue(S_OK));
  MOCKER(on_nested_msg_ack_2).expects(once()).will(returnValue(S_OK));

  Dispatcher::inst().registerAction(msg_start, makeMsgAction<Basic_mi_y>(defaultContext));
  Dispatcher::inst().handle_msg(msg_start);
  Dispatcher::inst().handle_msg(msg_ack_4);
  Dispatcher::inst().handle_msg(nested_msg_ack_1);
  Dispatcher::inst().handle_msg(nested_msg_ack_2);
}

//------------------------------------------------------------------------------
def_sync_call(send_msg_to_mgr);
def_sync_call(restart_sys);
def_sync_call(msg_ack_success);
def_sync_call(continue_more_works);

//def_mi(Basic_mi_gt) {
//    _step(sync_call_1);
//    
//    self------------>sysd---->async_step(send_msg_to_mgr) {
//      struct guard_task /*on_guard_task*/{
//	void setup_guards() {
//          self<------sysd<----ack(msg_ack_1, on_msg_ack_1); 
//	  self<------sysd<----ack(msg_ack_2, on_msg_ack_2);
//	}
//      } guard_task_inst;
//      guard_task_inst.setup_guards();
//      
//    } async_step_end;
//    
//} def_mi_end;

struct GuardTask {
  virtual void doTask() = 0;
};

typedef Status (*GuardTask_func) ();
typedef bool (*MsgPred_func) (const Msg& msg);

bool MsgPred_retire_if_rcv_success_msg (const Msg& msg) {
  return msg.id() == msg_succeeded;
}

bool guard_task_default_retire_pred_func(const Msg& unused)  {
  return true;
}

template< GuardTask_func guard_action
	, size_t guard_durationg_in_seconds
	, MsgPred_func should_retire_pred = guard_task_default_retire_pred_func> 
struct BaseGuard {
  virtual void doTask() {
    guard_action();
    return;
  }
  
  void activate_guard() {
    cout << "activate_guard by add to dispatcher, duration: "
	 << guard_durationg_in_seconds <<  endl;
  }

  static Status on_receive_ack_msg(const Msg& msg) {
    if (should_retire_pred(msg) && singleton_ != NULL) {
       cout << "guard::--------------> retire: " << msg.id() << "  ," << singleton_ << endl;
       return S_OK;
    }
    
    cout << "guard::--------------> on_receive_ack_msg: " << msg.id() << endl;
    return S_OK;
  }
  
  static BaseGuard* singleton_;
};

template< GuardTask_func guard_action
	, size_t guard_durationg_in_seconds
	, MsgPred_func should_retire_pred> 
BaseGuard<guard_action, guard_durationg_in_seconds, should_retire_pred>*
BaseGuard<guard_action, guard_durationg_in_seconds, should_retire_pred>::singleton_ = NULL;


Status resend_msg_to_mgr() {
  cout << "self------------------>mgr" << endl;
  return S_OK;  //TODO: return continue or remove_guard
}

bool retire_on_msg_succeeded_msg (const Msg& msg) {
  cout << "===================>: retire_on_msg_succeeded_msg" << endl;
  return msg.id() == msg_succeeded;
}

#define protected_by_guard(...) \
static struct guard : BaseGuard<resend_msg_to_mgr, _2s, retire_on_msg_succeeded_msg> { \
	  void setup_acks_handlers()

#define guard_end \
  } line_var(guard_inst); line_var(guard_inst).setup_acks_handlers(); line_var(guard_inst).activate_guard(); line_var(guard_inst).singleton_ = &line_var(guard_inst);


def_mi(Basic_mi_gt) {
    _step(sync_call_1);
    _step(sync_call_2);
    
    sysd<----async_step(send_msg_to_mgr) {
        protected_by_guard(send_msg_to_mgr, _2s, retire_on_msg_succeeded_msg) {
            sysd---->ack(msg_succeeded, continue_more_works);
	} guard_end;
	
	sysd---->ack(msg_failed, on_msg_ack_2);
    }                                                                              async_step_end;
}                                                                                  def_mi_end;

TEST_F(basic_mi_test, support_guard_task) {
  Dispatcher::inst().registerAction(msg_start, makeMsgAction<Basic_mi_gt>(defaultContext));
  Dispatcher::inst().handle_msg(msg_start);
  Dispatcher::inst().handle_msg(msg_succeeded);
  Dispatcher::inst().handle_msg(msg_failed);
}

//------------------------------------------------------------------------------
TEST_F(basic_mi_test, DISABLED_support_retire_guard_predicat_function) {
}
TEST_F(basic_mi_test, DISABLED_support_listener) {
}
//------------------------------------------------------------------------------
//todo: make name consistancy, like x and x_end
//        protected_by_guard(send_msg_to_mgr, _2s) {
//            self<------sysd<----ack(msg_succeeded, continue_more_works);
//	} protected_by_guard_end;


//def_mi(Basic_mi_gt) {
//    _step(sync_call_1);
//    _step(sync_call_2);
//    
//    self---------->sysd---->async_step(send_msg_to_mgr) {
//        protected_by_guard(send_msg_to_mgr, _2s) {
//            self<------sysd<----ack(msg_succeeded, continue_more_works111);
//            self<------sysd<----ack(msg_failed, continue_more_works222);
//	} guard_task_end;
//
//	self<------sysd<----ack(msg_succeeded, continue_more_works);
//    }                                                                              async_step_end;
//}                                                                                  def_mi_end;


#if 0
define_mi(PanelControlMI) {
   door---->ack___(EV_DOOR_OPENED , lock_panel);
   door---->ack___(EV_PANEL_CLOSED, lock_panel);

   door---->ack_ex(EV_DOOR_CLOSED) {
            step_maybe_return(wait_until___door_close___drawer_open__light_on);
            step__(unlock_panel);
   }

   door---->ack_ex(EV_DRAWER_OPEN) {
            step_maybe_return(wait_until___door_close___drawer_open__light_on);
            step__(unlock_panel);
   }

   door---->ack_ex(EV_LIGHT_ON) {
            step_maybe_return(wait_until___door_close___drawer_open__light_on);
            step__(unlock_panel);
   }
};

#endif

