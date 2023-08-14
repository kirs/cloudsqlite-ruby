Gem::Specification.new do |spec|
  spec.name          = "cloudsqlite-ruby"
  spec.version       = "0.1.0"
  spec.authors       = ["Kir Shatrov"]
  spec.email         = ["shatrov@me.com"]

  spec.summary       = "Example gem demonstrating a basic C extension."
  spec.description   = "Part of a project to explain how Ruby C extensions work."
  spec.homepage      = "https://github.com/flavorjones/ruby-c-extensions-explained"
  spec.required_ruby_version = ">= 2.4.0"
  spec.license = "MIT"

  # Specify which files should be added to the gem when it is released.
  # The `git ls-files -z` loads the files in the RubyGem that have been added into git.
  spec.files = Dir.chdir(File.expand_path(__dir__)) do
    `git ls-files -z`.split("\x0").reject { |f| f.match(%r{\A(?:test|spec|features)/}) }
  end
  # spec.bindir        = "exe"
  spec.executables   = []
  spec.require_paths = ["lib"]
  spec.extensions    = ["ext/cloudsqlite/extconf.rb"]

  # Uncomment to register a new dependency of your gem
  # spec.add_dependency "example-gem", "~> 1.0"

  # For more information and examples about making a new gem, checkout our
  # guide at: https://bundler.io/guides/creating_gem.html
end
