import os
import paramiko
from subprocess import call
import base64
import json
import threading
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

def get_and_build_vaultdb(user,address, repo_addr,ssh):
    print("Downloading from Github vaultdb on address %s" %(address))
    user_addr_str = user + "@" + address
    command = "git clone " + repo_addr
    run_command(user_addr_str, command)

    command = "cd vaultdb_operators"
    run_command(user,address, command)

    command = "./configure.sh"
    run_command(user,address, command)
    return

def run_command(user,address, command,ssh):

    ssh.connect(hostname = address, username=user, key_filename='/root/.ssh/id_rsa') # must have keys setup
    ssh_stdin,ssh_stdout,ssh_stderr = ssh.exec_command(command)

    return

def start_vaultdb_host(user,address, port, hb_info, ssh,experiment_number):

    print("Connecting to %s to start vault db"% (address))
    addr_port = address + ":" + port
    command = "./vaultdb " + "-honest_broker_address=" + hb_info + " -address=" + addr_port + " -experiment=" + experiment_number
    ssh_stdin,ssh_stdout,ssh_stderr = ssh.exec_command(command)
    # check the return??
    return


def start_vaultdb_hb(user,address, port, ssh, experiment_number):
    print("Connecting to %s with user %s to start vault db"% (address, user))

    addr_port = address + ":" + port
    command = "./vaultdb " + "-honest_broker=true " + " -address=" + addr_port + " -experiment=" + experiment_number

    ssh_stdin,ssh_stdout,ssh_stderr = ssh.exec_command(command)

    ssh_stdout.channel.recv_exit_status() # important line, this function won't return until honest broker is done...

    # check the return??
    return 1 # get something more useful from stdout

### SCRIPT STARTS HERE ###

BASE_DIR = os.getcwd()

try:
    os.chdir('cmake-build-debug')
except:
    os.mkdir('cmake-build-debug')
    os.chdir('cmake-build-debug')

call(['cmake','..'])
call(['make','clean'])
call(['make', 'vaultdb'])

# call(['cp', 'vaultdb', '..'])
# os.chdir('..')

ssh = paramiko.SSHClient()
ssh.load_system_host_keys() # loads current ssh authorized hosts/keys
ssh.set_missing_host_key_policy(paramiko.WarningPolicy) # if not found add
# ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())


with open(BASE_DIR + "/deploy.json") as json_data:
    data = json.load(json_data)

    # download and compile vaultdb on every machine
    hb_addr_port = data["HB"]["address"] + ":" + data["HB"]["port"]
    get_and_build_vaultdb(data["HB"]["user"], data["HB"]["address"])

    host_list = data["Hosts"]
    for host in host_list:
        get_and_build_vaultdb(host["user"], host["address"],ssh)


    experiments = data["Experiments"]
    for experiment in experiments:
        #make a thread to wait to until the experiment is finished
        tid = threading.Thread(target=start_vaultdb_hb,args=(data["HB"]["user"], data["HB"]["address"],data["HB"]["port"], experiment ))

        for host in host_list:
            start_vaultdb_host(host["user"], host["address"], host["port"], hb_addr_port, ssh, experiment)

        while(tid.isAlive()): # wait for the hb to return and finish
            print("waiting on hb")
            continue

        for host in host_list:
            kill_vaultdb(host["user"],host["address"],ssh)

    # start_vaultdb_hb(data["HB"]["user"], data["HB"]["address"],data["HB"]["port"])

