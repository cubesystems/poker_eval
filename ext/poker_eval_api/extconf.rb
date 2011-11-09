require 'mkmf'
find_header('poker_defs.h', '/usr/local/include/poker-eval')
create_makefile("poker_eval_api")
