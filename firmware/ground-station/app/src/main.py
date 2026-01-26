"""(script)
python script to start the rest api and load routes
"""

import sys
from typing import Final
from os.path import dirname, join

import uvicorn
from dotenv import load_dotenv

from core import GroundStation
from routes import router_list

PORT: Final = 8443
SSL_CERTFILE_PATH: Final = join(dirname(__file__), "cert.pem")
SSL_KEYFILE_PATH: Final = join(dirname(__file__), "key.pem")

# uvicorn for some reason requires app to be created at the module level
app = GroundStation(router_list)


def main():
    load_dotenv()

    options = {
        "app": "main:app",
        "host": "0.0.0.0",
        "reload": False,
        "port": PORT,
        "access_log": len(sys.argv) > 1 and sys.argv[1] == "log",
        "ssl_keyfile": SSL_KEYFILE_PATH,
        "ssl_certfile": SSL_CERTFILE_PATH,
    }

    uvicorn.run(**options)


if __name__ == "__main__":
    main()
