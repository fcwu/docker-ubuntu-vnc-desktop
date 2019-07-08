import os, fnmatch

toFind = "/app"
toReplace = os.getenv("PREFIX_PATH")

if (toReplace is None):
    print("No path given by environment variable.")
    exit()

print("using path {0} ...".format(toReplace))

def findReplace(directory, find, replace):
    print("Rename {0} into {1} at {2}".format(toFind, toReplace, directory))
    for path, dirs, files in os.walk(os.path.abspath(directory)):
        for filename in files:
            filepath = os.path.join(path, filename)
            with open(filepath) as f:
                s = f.read()
            s = s.replace(find, replace)
            with open(filepath, "w") as f:
                f.write(s)

findReplace("/etc/nginx/sites-enabled/", toFind, toReplace)