#!/usr/local/bin/python
from jinja2 import Environment, FileSystemLoader
from os import mkdir, path
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("--desktop", help="Accepts 'enabled' or 'disabled'")
args = parser.parse_args()

# Default Nginx
environment = Environment(loader=FileSystemLoader("./"))
template = environment.get_template("nginx_default.j2")
filename = f"../rootfs/etc/nginx/sites-enabled/default"
content = template.render(
    desktop=args.desktop,
)
with open(filename, mode="w", encoding="utf-8") as message:
    message.write(content)
    print(f"... wrote {filename}")
    
    
# Supervisor Conf
environment = Environment(loader=FileSystemLoader("./"))
template = environment.get_template("supervisord.conf.j2")
filename = f"../rootfs/etc/supervisor/conf.d/supervisord.conf"

content = template.render(
    desktop=args.desktop,
)
with open(filename, mode="w", encoding="utf-8") as message:
    message.write(content)
    print(f"... wrote {filename}")