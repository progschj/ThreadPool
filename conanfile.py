from conans import ConanFile


class ThreadPool(ConanFile):
    name = "ThreadPool"
    url = "https://github.com/sztomi/ThreadPool"
    license = "MIT"
    version = "1.0.0"
    exports = "*.h"
    build_policy = "missing"

    def package(self):
        self.copy("*.h", src=".", dst="include")
