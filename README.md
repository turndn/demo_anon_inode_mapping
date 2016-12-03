# Demo, anonymous inode mapping to userspace
Anonymous inode を作って構造体をマップして userspace からアクセスするやつ．

# Usage

    $ make
    $ make user
    $ make insmod
    sudo insmod mapping_module.ko
    [sudo] password for up: 
    # ./user
    [+] Open /dev/up
    [+] Create struct_child at LKM
    [+] Get mmap_size
    [+] Create anonymous inode
    [+] Map child_struct
    child_struct: id=1, value=10
    [+] Change child_struct->value
    child_struct: id=1, value=5
