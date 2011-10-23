require 'rubygems/package_task'
require 'hoe'

HOE = Hoe.spec 'poker-eval' do
  self.author         = ['Miks Mikelsons']
  self.email          = %w[miks@cubesystems.lv]
  self.need_tar       = false
  self.need_zip       = false

  spec_extras[:required_ruby_version] = Gem::Requirement.new('>= 1.9.2')

  spec_extras[:extensions] = ["ext/poker-eval/extconf.rb"]

  extra_dev_deps << ['rake-compiler', "~> 0.5"]
end

file "#{HOE.spec.name}.gemspec" => ['Rakefile', 'tasks/gem.rake'] do |t|
  puts "Generating #{t.name}"
  File.open(t.name, 'w') { |f| f.puts HOE.spec.to_yaml }
end

desc "Generate or update the standalone gemspec file for the project"
task :gemspec => ["#{HOE.spec.name}.gemspec"]
