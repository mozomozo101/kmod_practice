/*
 * sudo insmod proc_rw.ko 
 * ls /proc/hello 	//procファイル生成
 * echo unko > /proc/hello // procファイルへ書き込み
 * cat /proc/hello	// 読み込み
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include <linux/slab.h>

int len,temp;

char *msg;

// ssize_t (*read) (struct file *, char __user *, size_t, loff_t *);
static int read_proc(struct file *filp,char *buf,size_t count,loff_t *offp ) 
{
	if(count>temp)
	{
		count=temp;
	}
	temp=temp-count;
	copy_to_user(buf,msg, count);	// catのプロセス空間bufにデータコピー
	if(count==0)					// ここで疑問。msgとprocファイルはどう結びついてるのか？
		temp=len;

	return count;
}

//write( fd, バッファ, サイズ)
// ssize_t (*write) (struct file *, const char __user *, size_t, loff_t *);
static int write_proc(struct file *filp,const char *buf,size_t count,loff_t *offp)
{
	// copy_from_user: 
	// ユーザ空間のデータをカーネル空間に転送
	// copy_from_user( to, from, length)
	copy_from_user(msg,buf,count);
	len=count;
	temp=len;
	return count;
}

// モジュールAPIの登録
static struct file_operations proc_fops = {
	owner:THIS_MODULE,
	read: read_proc,
	write: write_proc
};
static void create_new_proc_entry() 
{
	struct proc_dir_entry *new_proc, *root;
	new_proc = proc_create("dummy", 0644, NULL, &proc_fops);
	//root = new_proc->parent;
	printk( KERN_INFO "proc_root: %s\n", new_proc->mode );
	
	//struct proc_dir_entry *proc_sys_kernel;
	//proc_sys = proc_mkdir("sys", NULL);
	

	proc_create("miracle", 0644, NULL, &proc_fops);



	//proc_create("sys/kernel/miracle",0666,NULL,&proc_fops);

	// モジュール読み込みの時点で、バッファを確保しておく
	msg=kmalloc(GFP_KERNEL,10*sizeof(char));
}


static int __init proc_init (void) {
	create_new_proc_entry();
	return 0;
}

static void __exit proc_cleanup(void) {
	remove_proc_entry("hello",NULL);
}

MODULE_LICENSE("GPL"); 
module_init(proc_init);
module_exit(proc_cleanup);

