from kfp import dsl
from kfp import compiler


@dsl.container_component
def say_hello(name: str, greeting: dsl.OutputPath(str)):  # type: ignore
    return dsl.ContainerSpec(
        image="alpine",
        command=[
            "sh",
            "-c",
            """RESPONSE="echo Hello, $0!" \
                 && echo $RESPONSE \
                 && mkdir -p $(dirname $1) \
                 && echo $RESPONSE > $1
                 """,
        ],
        args=[name, greeting],
    )


@dsl.pipeline
def hello_pipeline(person: str) -> str:
    task = say_hello(name=person)
    return task.outputs["greeting"]


compiler.Compiler().compile(hello_pipeline, "pipeline_container_hello.yaml")
