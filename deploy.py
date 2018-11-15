import os
import paramiko
from subprocess import call
import base64
import json
import time
import threading
import sys
### HELPER FUNCTIONS ###

# TODO: test this on other machines.
# TODO: Have this use ssh keys not just passwords
# TODO: stop using root
# TODO: confirm that text file busy is because using localhost... ?
# custom sink for teh logger write back rpc calls to honest broker.

def get_and_build_vaultdb(user,address, repo_addr,ssh, install_dir):
    print("Downloading from Github vaultdb on address %s" %(address))
    command = "git clone " + repo_addr + " " + install_dir + "/vaultdb_operators" # make new folder for vaultdb
    i,out,err = run_command_blocking(command,ssh)
    print(err.readlines())
    print("Downloaded Github Repo")

    command = "cd " + install_dir +  "/vaultdb_operators"
    i,out,err = run_command_blocking(command,ssh)
    print(err.readlines())

    print("configuring")
    command = install_dir + "/vaultdb_operators/configure.sh"
    i,out,err = run_command_blocking(command,ssh)
    print(err.readlines())
    print("done configuring")

    return

def run_command_blocking(command,ssh):
    ssh_stdin, ssh_stdout, ssh_stderr = ssh.exec_command(command)
    ssh_stdout.channel.recv_exit_status() # important line, this function won't return until honest broker is done...

    return ssh_stdin, ssh_stdout, ssh_stderr


def run_command(user,address, command,ssh):

    ssh_stdin,ssh_stdout,ssh_stderr = ssh.exec_command(command)

    return ssh_stdin,ssh_stdout,ssh_stderr

def kill_vaultdb(user,address,ssh):
    command = "killall -9 vaultdb"
    run_command(user,address,command,ssh)
    return


def start_vaultdb_host(user,address, port, hb_info, ssh, experiment_number, base_dir):

    print("Connecting to %s to start vault db"% (address))
    addr_port = address + ":" + port
    command = "LD_LIBRARY_PATH=~/vaultdb_operators/lib/lib " + base_dir + "vaultdb_operators/build/vaultdb " + "-honest_broker_address=" + hb_info + " -address=" + addr_port #+ " -experiment=" + experiment_number
    ssh_stdin,ssh_stdout,ssh_stderr = run_command(user,address,command,ssh)
    # check the return?
    return ssh_stdin,ssh_stdout,ssh_stderr


def start_vaultdb_hb(user,address, port, ssh, experiment_number, num_hosts, base_dir):
    print("Connecting to %s with user %s to start vault db"% (address, user))
    print("THREAD!")

    addr_port = address + ":" + port                                    # assuming that we build it in vaultdb_operators and configure is run
    command = "LD_LIBRARY_PATH=~/vaultdb_operators/lib/lib " + base_dir +  "vaultdb_operators/build/vaultdb " + "-honest_broker=true " + " -address=" + addr_port #+ " -experiment=" + experiment_number + " -expected_num_hosts=" + num_hosts

    ssh_stdin,ssh_stdout,ssh_stderr = run_command(user,address,command,ssh)

    ssh_stdout.channel.recv_exit_status() # important line, this function won't return until honest broker is done...

    # check the return??
    return 1 # get something more useful from stdout

### SCRIPT STARTS HERE ###

BASE_DIR = os.getcwd()

def new_ssh():
    ssh = paramiko.SSHClient()
    ssh.load_system_host_keys() # loads current ssh authorized hosts/keys
    ssh.set_missing_host_key_policy(paramiko.WarningPolicy) # if not found add
    # ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    return ssh

ssh_queue = []


with open(BASE_DIR + "/deploy.json") as json_data:
    data = json.load(json_data)
    host_list = data["Hosts"]
    experiments = data["Experiments"]
    github_repo = data["Github"]
    base_dir = data["BASE_DIR"]
    # download and compile vaultdb on every machine
    hb_addr_port = data["HB"]["address"] + ":" + data["HB"]["port"]

    # for all hosts make ssh client
    for host in host_list:
        new_ssh = paramiko.SSHClient()
        new_ssh.load_system_host_keys() # loads current ssh authorized hosts/keys
        new_ssh.set_missing_host_key_policy(paramiko.WarningPolicy) # if not found add
        ssh_queue.append(new_ssh)
        new_ssh.connect(hostname = host["address"], username=host["user"]) # must have keys setup

        get_and_build_vaultdb(host["user"], host["address"],github_repo,new_ssh, base_dir)


    sys.exit(0)

    for experiment in experiments:
        #make a thread to wait to until the experiment is finished

        ssh_hb = paramiko.SSHClient()
        ssh_hb.load_system_host_keys() # loads current ssh authorized hosts/keys
        ssh_hb.set_missing_host_key_policy(paramiko.WarningPolicy) # if not found add

        num_hosts = len(host_list)

        tid = threading.Thread(target=start_vaultdb_hb,args=(data["HB"]["user"], data["HB"]["address"],data["HB"]["port"], ssh_hb, experiment,num_hosts, base_dir ))
        tid.start()
        time.sleep(2)
        for host in host_list:
            new_ssh = paramiko.SSHClient()
            new_ssh.load_system_host_keys() # loads current ssh authorized hosts/keys
            new_ssh.set_missing_host_key_policy(paramiko.WarningPolicy) # if not found add
            ssh_queue.append(new_ssh)
            a,b,c = start_vaultdb_host(host["user"], host["address"], host["port"], hb_addr_port, new_ssh, experiment, base_dir)




        while(tid.isAlive()): # wait for the hb to return and finish
            #print("waiting on hb")
            continue


        for host in host_list: # call killall (probably only need to do it once)
            new_ssh = paramiko.SSHClient()
            new_ssh.load_system_host_keys() # loads current ssh authorized hosts/keys
            new_ssh.set_missing_host_key_policy(paramiko.WarningPolicy) # if not found add
            run_command(host["user"],host["address"], "killall -9 vaultdb", new_ssh)


