# support multiple ruby version (fat binaries under windows)
begin
  require 'poker_eval_api'
rescue LoadError
  if RUBY_PLATFORM =~ /mingw|mswin/ then
    RUBY_VERSION =~ /(\d+.\d+)/
    require "#{$1}/poker-eval"
  end
end

# define version string to be used internally for the Gem by Hoe.
class PokerEval
  module GemVersion
    VERSION = '0.0.1'
  end
end

