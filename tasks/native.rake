# use rake-compiler for building the extension
require 'rake/extensiontask'

Rake::ExtensionTask.new('poker_eval_api', HOE.spec) do |ext|

  # automatically add build options to avoid need of manual input
  # ext.cross_compile = true
  ext.cross_config_options << "--with-poker-eval-include=/usr/local/include"
  ext.cross_config_options << "--with-poker-eval-lib=/usr/local/lib/opt"
end

# ensure things are compiled prior testing
task :test => [:compile]
