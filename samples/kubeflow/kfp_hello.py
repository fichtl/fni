from kfp import dsl
from kfp import compiler


@dsl.component(base_image="python:3.8")
def say_hello(name: str) -> str:
    hello_text = f"hello_{name}"
    print(hello_text)
    return hello_text


@dsl.pipeline
def hello_pipeline(recipent: str) -> str:
    hello_task = say_hello(name=recipent)
    return hello_task.output


compiler.Compiler().compile(hello_pipeline, "pipeline_hello.yaml")
