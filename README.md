# Demo, anonymous inode mapping to userspace
This LKM creates anonymous inode which is accessed from userspace.
A structure is mapped to an allocated page.

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
