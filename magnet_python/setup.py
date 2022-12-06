from setuptools import setup

requirements = [
    'pyserial',
]

setup(
    name="esp32-magnet",
    version="1.0.0",
    author="ClaireChen",
    keywords=["electromagnet", "ESP32"],
    author_email="2905415904@qq.com",
    description="A toolkit to trigger electromagnet",
    long_description=open('README.md').read(),
    long_description_content_type="text/markdown",
    license="MIT Licence",
    packages=["esp32_magnet"],
    python_requires=">=3.6",
    install_requires=requirements
)
