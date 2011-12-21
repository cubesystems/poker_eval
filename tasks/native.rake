# use rake-compiler for building the extension
require 'rake/extensiontask'

Rake::ExtensionTask.new('poker_eval_api', $gemspec) do |ext|
  ext.cross_compile   = true

  # automatically add build options to avoid need of manual input
  # ext.cross_compile = true
  ext.cross_config_options << "--with-poker-eval-include=/usr/local/include"
  ext.cross_config_options << "--with-poker-eval-lib=/usr/local/lib/opt"
end

CLEAN.include 'lib/**/*.so'

# ensure things are compiled prior testing
task :test => [:compile]

# Workaround for rake-compiler, which YAML-dump-loads the
# gemspec, which leads to errors since Procs can't be loaded
Rake::Task.tasks.each do |task_name|
    case task_name.to_s
    when /^native/
        task_name.prerequisites.unshift("fix_rake_compiler_gemspec_dump")
    end
end

task :fix_rake_compiler_gemspec_dump do
    %w{files extra_rdoc_files test_files}.each do |accessor|
        $gemspec.send(accessor).instance_eval { @exclude_procs = Array.new }
    end
end

