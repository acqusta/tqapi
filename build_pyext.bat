set VS90COMNTOOLS=%VS140COMNTOOLS%

python setup.py build_clib --compiler=msvc
python setup.py build_ext --compiler=msvc
python setup.py sdist bdist_wheel