from os.path import dirname, join

from fastapi import APIRouter
from fastapi.responses import FileResponse

site_router = APIRouter(
    tags=[
        "Site",
    ],
)


@site_router.get("/", response_class=FileResponse)
def home():
    return FileResponse(join(dirname(__file__), "../../static/index.html"))
