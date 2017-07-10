#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/anon_inodes.h>
#include <linux/ioctl.h>
#include <linux/fs.h>
#include <linux/mm.h>

MODULE_AUTHOR("up");
MODULE_DESCRIPTION("Anonymous inode mapping to userspace");
MODULE_LICENSE("GPL");

#define DEV_MINOR DMAPI_MINOR /* unused */

#define UPIO		0xAF
#define CMD_CREATE 	_IO(UPIO,	0x00)
#define CMD_OPEN 	_IO(UPIO,	0x01)
#define CMD_VAL  	_IOW(UPIO,	0x02, int)
#define CMD_MMAPSIZE	_IO(UPIO,	0x03)

struct child_struct {
	__u32 id;
	__s32 value;
};

struct parent_struct {
	__u32 id;
	struct child_struct *child;
};

static long parent_dev_ioctl(struct file *flip,
				unsigned int ioctl, unsigned long arg);

struct parent_struct parent = {
	.id = 1,
	.child = NULL,
};

static int child_fault(struct vm_fault *vmf)
{
	struct parent_struct *p = vmf->vma->vm_file->private_data;
	struct page *page;

	if (vmf->pgoff == 0)
		page = virt_to_page(p->child);
	else
		return -1;

	printk(KERN_INFO "mapping_module: child id:%u, value:%d\n",
		p->child->id, p->child->value);
	get_page(page);
	vmf->page = page;
	return 0;
}

static const struct vm_operations_struct child_vm_ops = {
	.fault = child_fault,
};

static int child_mmap(struct file *file, struct vm_area_struct *vma)
{
	vma->vm_ops = &child_vm_ops;
	return 0;
}

static struct file_operations child_fops = {
	.mmap 	= child_mmap,
	.llseek = noop_llseek,
};

static struct file_operations parent_fops = {
	.unlocked_ioctl = parent_dev_ioctl,
	.llseek = noop_llseek,
};

static struct miscdevice parent_dev = {
	DEV_MINOR,
	"up",
	&parent_fops,
};

static long parent_dev_ioctl(struct file *filp, 
				unsigned int ioctl, unsigned long arg)
{
	long r = -EINVAL;
	struct page *page;

	switch (ioctl) {
	case CMD_CREATE:
		page = alloc_page(GFP_KERNEL | __GFP_ZERO);
		if (!page) {
			r = -ENOMEM;
			goto done;
		}
		parent.child = page_address(page);
		parent.child->id = 1;
		parent.child->value = 10;
		r = 1;
		break;
	case CMD_OPEN:
		if (parent.child == NULL)
			goto done;

		r = anon_inode_getfd("up-child", &child_fops,
					&parent, O_RDWR | O_CLOEXEC);
		break;
	case CMD_VAL:
		if (parent.child == NULL)
			goto done;

		parent.child->value = arg;
		r = 1;
		break;
	case CMD_MMAPSIZE:
		r = PAGE_SIZE;
		break;
	default:
		break;
	}

done:
	return r;
}

static int __init mapping_module_init(void)
{
	int r;

	r = misc_register(&parent_dev);
	if (r) {
		printk(KERN_ERR "mapping_module: misc device register failed\n");
		return r;
	}

	printk(KERN_INFO "mapping_module: registered /dev/up\n");

	return 0;
}
module_init(mapping_module_init);

static void __exit mapping_module_exit(void)
{
	if (parent.child)
		free_page((unsigned long)parent.child);

	misc_deregister(&parent_dev);
	printk(KERN_INFO "mapping_module: deregistered /dev/up\n");
}
module_exit(mapping_module_exit);
