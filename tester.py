import os
import sys
import unittest

ROOT = os.path.dirname(os.path.abspath(__file__))
DIRECTORY = os.path.join(ROOT, 'Source', 'PythonTests')
CONFIGURATIONS = ('Debug', 'Release', 'RelWithDebInfo', 'MinSizeRel')
MODULES = ('aspen.pyd', 'aspen.so')


def _roots():
  roots = []
  for root in (os.getcwd(), ROOT):
    root = os.path.abspath(root)
    if root not in roots:
      roots.append(root)
  return roots


def _built_module():
  for root in _roots():
    modules = []
    for configuration in CONFIGURATIONS:
      for module in MODULES:
        path = os.path.join(root, 'Libraries', configuration, module)
        if os.path.isfile(path):
          modules.append(path)
    if modules:
      return max(modules, key=os.path.getmtime)
  return None


def main():
  if len(sys.argv) > 1:
    pattern = '{}*.py'.format(sys.argv[1])
  else:
    pattern = '*_tester.py'
  module = _built_module()
  if module:
    sys.path.insert(0, os.path.dirname(module))
  sys.path.insert(0, DIRECTORY)
  import aspen
  print('Testing {}'.format(aspen.__file__), flush=True)
  tests = unittest.defaultTestLoader.discover(DIRECTORY, pattern=pattern)
  if tests.countTestCases() == 0:
    print('No tests matched {}.'.format(pattern))
    return 1
  result = unittest.TextTestRunner(verbosity=2).run(tests)
  return 0 if result.wasSuccessful() else 1


if __name__ == '__main__':
  sys.exit(main())
