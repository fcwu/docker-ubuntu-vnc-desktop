Failing to generate a key using the below path will cause the key to delete after a routine update to the cluster. After the upgrade, use the command above to generate a new key for it to be persistent

Generate percistent SSH key using otherwise it will delete on upgrade:
#ssh-keygen -t ed25519 -f /workspace/.%USER%/.ssh/id_ed25519 -C "%USER%@%DOMAIN%"

