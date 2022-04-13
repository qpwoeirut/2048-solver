from pybind11.setup_helpers import Pybind11Extension, build_ext
from setuptools import setup

ext_modules = [
    Pybind11Extension("players", ["demo/selenium/export_players.cpp"]),
]

setup(
    name="players",
    author="Stanley Zhong",
    description="Solvers for 2048",
    long_description="Solving strategy implementations in C++, exported to Python",
    ext_modules=ext_modules,
    extras_require={"test": "pytest"},
    cmdclass={"build_ext": build_ext},
    zip_safe=False,
    python_requires=">=3.6",
)
