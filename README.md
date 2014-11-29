reactive_cpp
============

this repo shows how to implement an DSL to describe reactive application.

```
def_mi(Basic_mi_x) {
                                         step(sync_call_1);
                                         step(sync_call_2);
        sysd<----------------------------async_step(async_call_1) {
        	sysd-------------------->ack(msg_ack_1, on_msg_ack_1);
        	sysd-------------------->ack(msg_ack_2, on_msg_ack_2);
                
        	sysd-------------------->ack_ex(msg_ack_4)
                {
                                         step(sync_call_nested_1);
                        sysd<------------async_step(nested_async_call_1)
                        {
                                 sysd---->ack(nested_msg_ack_1, on_nested_msg_ack_1);
                                 sysd---->ack(nested_msg_ack_2, on_nested_msg_ack_2);
                                 sysd---->ack(msg_ack_1, on_nested_msg_ack_3);            
                        } async_step_end;
        	} on_msg_ack_ex_end;
        }async_step_end;
} def_mi_end;


```
