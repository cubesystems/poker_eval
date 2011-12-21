# encoding: UTF-8
task :release do
    sh "vim HISTORY.markdown"
    sh "vim README.markdown"
    sh "git commit -a -m 'prerelease adjustments'; true"
end

require 'jeweler'
jeweler_tasks = Jeweler::Tasks.new do |gem|
    gem.name                = 'poker_eval'
    gem.summary             = 'poker evaluation Ruby Interface'
    gem.description         = gem.summary
    gem.email               = 'miks@cube.lv'
    gem.homepage            = 'http://cubesystems.lv'
    gem.authors             = ['Miks Miķelsons']
    gem.extensions          = FileList['ext/**/extconf.rb']

    gem.files.include('lib/poker_eval.*') # add native stuff
end

$gemspec         = jeweler_tasks.gemspec
$gemspec.version = jeweler_tasks.jeweler.version

Jeweler::RubyforgeTasks.new
Jeweler::GemcutterTasks.new

