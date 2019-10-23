set VS90COMNTOOLS=%VS140COMNTOOLS%

del setup.cfg
python setup.py sdist

copy /y setup.cfg.win32 setup.cfg

python setup.py build_clib --compiler=msvc
python setup.py build_ext --compiler=msvc
python setup.py bdist_wheel

