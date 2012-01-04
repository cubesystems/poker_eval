# Generated by jeweler
# DO NOT EDIT THIS FILE DIRECTLY
# Instead, edit Jeweler::Tasks in Rakefile, and run 'rake gemspec'
# -*- encoding: utf-8 -*-

Gem::Specification.new do |s|
  s.name = "poker_eval"
  s.version = "0.0.2"

  s.required_rubygems_version = Gem::Requirement.new(">= 0") if s.respond_to? :required_rubygems_version=
  s.authors = ["Miks Mikelsons"]
  s.date = "2012-01-04"
  s.description = "poker evaluation Ruby Interface"
  s.email = "miks@cube.lv"
  s.extensions = ["ext/poker_eval_api/extconf.rb"]
  s.extra_rdoc_files = [
    "README.txt"
  ]
  s.files = [
    "COPYING",
    "COPYING.ja",
    "Gemfile",
    "History.txt",
    "README.txt",
    "Rakefile",
    "VERSION",
    "ext/poker_eval_api/extconf.rb",
    "ext/poker_eval_api/poker_eval.c",
    "lib/poker_eval.rb",
    "poker_eval.gemspec",
    "tasks/jeweler.rake",
    "tasks/native.rake",
    "test/test_poker_eval.rb"
  ]
  s.homepage = "http://cubesystems.lv"
  s.require_paths = ["lib"]
  s.rubygems_version = "1.8.10"
  s.summary = "poker evaluation Ruby Interface"

  if s.respond_to? :specification_version then
    s.specification_version = 3

    if Gem::Version.new(Gem::VERSION) >= Gem::Version.new('1.2.0') then
      s.add_runtime_dependency(%q<jeweler>, [">= 0"])
      s.add_runtime_dependency(%q<rake-compiler>, [">= 0"])
    else
      s.add_dependency(%q<jeweler>, [">= 0"])
      s.add_dependency(%q<rake-compiler>, [">= 0"])
    end
  else
    s.add_dependency(%q<jeweler>, [">= 0"])
    s.add_dependency(%q<rake-compiler>, [">= 0"])
  end
end

