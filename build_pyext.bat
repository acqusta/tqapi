set VS90COMNTOOLS=%VS140COMNTOOLS%

copy /y setup.cfg.win32 setup.cfg

python setup.py build_clib --compiler=msvc
python setup.py build_ext --compiler=msvc
python setup.py sdist bdist_wheel