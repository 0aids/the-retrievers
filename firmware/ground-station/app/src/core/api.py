"""(module) api
This contains the API class (FastAPI subclass)
"""

from os.path import dirname, join
from contextlib import asynccontextmanager

from rich import print
from fastapi import FastAPI, APIRouter
from fastapi.staticfiles import StaticFiles
from fastapi.middleware.cors import CORSMiddleware


class GroundStation(FastAPI):
    def __init__(self, router_list: list[APIRouter]) -> None:
        super().__init__(
            title="Ground Station REST API",
            description="type shit",
            lifespan=self.__lifespan,
        )

        for route in router_list:
            self.include_router(router=route)

        cors_options = {
            "allow_origins": ["*"],
            "allow_methods": ["*"],
            "allow_headers": ["*"],
            "allow_credentials": True,
        }
        self.add_middleware(CORSMiddleware, **cors_options)

        self.mount(
            "/static",
            StaticFiles(directory=join(dirname(__file__), "../../static")),
            name="static",
        )

    @staticmethod
    @asynccontextmanager
    async def __lifespan(app: FastAPI):
        print("[bold blue]API has started!")
        yield
        print("[bold blue]API has been shutdown!")
