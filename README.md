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
                        sysx<------------async_step(nested_async_call_1)
                        {
                                 sysx---->ack(nested_msg_ack_1, on_nested_msg_ack_1);
                                 sysx---->ack(nested_msg_ack_2, on_nested_msg_ack_2);
                                 sysx---->ack(msg_ack_1, on_nested_msg_ack_3);            
                        } async_step_end;
        	} on_msg_ack_ex_end;
        }async_step_end;
} def_mi_end;

```


```
j@j:~/lab/msgflow$ cat input.txt
#!MF:regex:.*\[(\w+)\].*---->.*\[(\w+)\] (.+) (.*?), #!MF:reformat_to:src:@1, dst:@2, msg_id:@3, extra_info:[@4]
#!MF:main_actor:self
#!MF:unknwn_msg_as_extra_info:
#!MF:draw_from_right:
[self] ---->   [sysd] EV_async_call_1 0x01010101
[sysd] ---->   [self] msg_ack_4 0x01010101
[self] ---->   [sysx] EV_nested_async_call_1 0x01010101
[sysx] ---->   [self] msg_ack_1 0x01010101
```


```
j@j:~/lab/msgflow$ ./msgflow < input.txt
sysx   sysd   self
  |      |<-----|     EV_async_call_1  
  |      |----->|     msg_ack_4   
  |<------------|     EV_nested_async_call_1   
  |------------>|     msg_ack_1   
```
