echo -n Password:
read -s PASSWORD
#PASSWORD="" #password"
echo $PASSWORD | sudo -S apt-get update
echo $PASSWORD | sudo -S apt-get -y install build-essential autoconf libtool pkg-config cmake git clang libgoogle-perftools-dev ocaml ocamlbuild libgoogle-perftools-dev postgresql-all
