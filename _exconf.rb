require 'mkmf'
# have_library("poker-eval")
find_header('poker_defs.h', '/usr/local/include/poker-eval')
dir_config("pokereval")
create_makefile("pokereval")
