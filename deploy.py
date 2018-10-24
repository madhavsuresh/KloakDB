import os
import paramiko
from subprocess import call
import base64
import json

### HELPER FUNCTIONS ###

# TODO: test this on other machines.
# TODO: Have this use ssh keys not just passwords 
# TODO: stop using root
# TODO: confirm that text file busy is because using localhost... ?
# custom sink for teh logger write back rpc calls to honest broker.

def copy_vaultdb(user,address):
    print("Copying vaultdb executable over to host address %s" %(address))
    user_addr_str = user + "@" + address + ":~" # scp to home dir
    call(['scp', '-i', '~/.ssh/id_rsa','vaultdb', user_addr_str])
    return    

def start_vaultdb_host(user,address, port, hb_info):

    print("Connecting to %s to start vault db"% (address))
    ssh = paramiko.SSHClient()
    ssh.load_system_host_keys()
    ssh.set_missing_host_key_policy(paramiko.WarningPolicy)
    # ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    ssh.connect(hostname = address, username=user, key_filename='/root/.ssh/id_rsa', password='') # ideally get keys working
    addr_port = address + ":" + port
    command = "./vaultdb " + "-honest_broker_address=" + hb_info +  " -address=" + addr_port
    ssh_stdin,ssh_stdout,ssh_stderr = ssh.exec_command(command)
    # check the return??
    return


def start_vaultdb_hb(user,address, port):
    print("Connecting to %s with user %s to start vault db"% (address, user))
    
    ssh = paramiko.SSHClient()
    ssh.load_system_host_keys()
    ssh.set_missing_host_key_policy(paramiko.WarningPolicy)
    # ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    ssh.connect(hostname = address, username=user, key_filename='/root/.ssh/id_rsa', password='') # ideally get keys working
    
    addr_port = address + ":" + port
    command = "./vaultdb " + "-honest_broker=true " + "-address=" + addr_port

    ssh_stdin,ssh_stdout,ssh_stderr = ssh.exec_command(command)
    # check the return??
    return addr_port # this will be useful for other commands


### SCRIPT STARTS HERE ###

try:
    os.chdir('cmake-build-debug')
except:
    print("dir structure is wrong, cmake-build-debug isn't rhere")
    raise Exception("where is cmake")

call(['make','clean'])
call(['make', 'vaultdb'])

call(['cp', 'vaultdb', '..'])
os.chdir('..')

with open('deploy.json') as json_data:
    data = json.load(json_data)
    
    honest_broker_addr = data["HB"]["address"] + ":" + data["HB"]["port"]

    copy_vaultdb(data["HB"]["user"], data["HB"]["address"])
    hb_addr_port = start_vaultdb_hb(data["HB"]["user"], data["HB"]["address"],data["HB"]["port"])


    host_list = data["Hosts"]
    for host in host_list:                                         
        
        copy_vaultdb(host["user"], host["address"])
        start_vaultdb_host(host["user"], host["address"], host["port"], hb_addr_port)
                

