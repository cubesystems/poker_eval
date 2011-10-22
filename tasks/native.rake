# use rake-compiler for building the extension
require 'rake/extensiontask'

MYSQL_VERSION = "5.0.83"
MYSQL_MIRROR  = ENV['MYSQL_MIRROR'] || "http://mysql.localhost.net.ar"

Rake::ExtensionTask.new('poker_eval', HOE.spec) do |ext|
  # reference where the vendored MySQL got extracted
  # mysql_lib = File.expand_path(File.join(File.dirname(__FILE__), '..', 'vendor', "mysql-#{MYSQL_VERSION}-win32"))


  # automatically add build options to avoid need of manual input
  # ext.cross_compile = true
  ext.cross_config_options << "--with-poker_eval-include=/usr/local/include"
  ext.cross_config_options << "--with-poker_eval-lib=/usr/local/lib/opt"
end

# ensure things are compiled prior testing
task :test => [:compile]
