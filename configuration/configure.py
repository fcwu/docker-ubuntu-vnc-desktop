from jinja2 import Environment, FileSystemLoader
from os import mkdir, path
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("--desktop", help="Accepts 'enabled' or 'disabled'")
parser.add_argument("--ubuntu", help="Accepts '20' or '22'")
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
    
    
# Supervisor Conf
environment = Environment(loader=FileSystemLoader("./"))
template = environment.get_template("Dockerfile.j2")
filename = f"../Dockerfile"

content = template.render(
    version=args.ubuntu,
)
with open(filename, mode="w", encoding="utf-8") as message:
    message.write(content)
    print(f"... wrote {filename}")