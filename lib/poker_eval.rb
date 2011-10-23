# support multiple ruby version (fat binaries under windows)
begin
  require 'poker_eval_api'
end

# define version string to be used internally for the Gem by Hoe.
class PokerEval
  module GemVersion
    VERSION = '0.0.1'
  end
end

