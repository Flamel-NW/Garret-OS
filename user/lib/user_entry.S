.text
.global user_entry
user_entry:
    # call user program function
    call user_main
1:  j 1b
