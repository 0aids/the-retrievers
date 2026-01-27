__all__ = ("router_list", "middleware_list")

from .site import site_router
from .api import command_router, state_router

router_list = [site_router, command_router, state_router]
middleware_list = []
