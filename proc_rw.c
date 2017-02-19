/*
 * sudo insmod proc_rw.ko 
 * ls /proc/hello 	//procファイル生成
 * echo unko > /proc/hello // procファイルへ書き込み
 * cat /proc/hello	// 読み込み
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include <linux/slab.h>
#include "../kernel/sched/sched.h"
#include <linux/rbtree.h>

int len,temp;

char *msg;

// モジュールAPIの登録
struct file_operations proc_fops = {
	owner:THIS_MODULE,
	read: read_proc,
	write: write_proc
};

// ランキューの取得〜ルートノード取得
struct rb_node *get_root(void){
	
	int cpu = smp_processor_id();
	struct rq *rq = cpu_rq(cpu);
	struct rb_node *root = (rq->cfs).tasks_timeline.rb_node;
	return root;
}

// ノードに対応したタスク優先度取得
void get_priority(struct rb_node *node){
	
	struct sched_entity *s_entity;
	struct task_struct *task;

	s_entity = rb_entry(node, struct sched_entity, run_node);	// sched_entry構造体取得
	task = container_of(s_entity, struct task_struct, se);		// task構造体取得
	printk( KERN_INFO "task_priority:%d", task->prio );
}

// 二分木走査
void rb_scan(struct rb_node *node){
	get_priority(node);
	rb_scan(node->rb_left);
	rb_scan(node->rb_right);
}

// 処理の取りまとめ関数
void count_process(void){
	struct rb_node *root = get_root();
	rb_scan(root);
}


int read_proc(struct file *filp,char *buf,size_t count,loff_t *offp ) 
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
int write_proc(struct file *filp,const char *buf,size_t count,loff_t *offp)
{
	// copy_from_user: 
	// ユーザ空間のデータをカーネル空間に転送
	// copy_from_user( to, from, length)
	copy_from_user(msg,buf,count);
	len=count;
	temp=len;
	return count;
}


int proc_init (void) {
	proc_create("hello",0666,NULL,&proc_fops);
	
	// モジュール読み込みの時点で、バッファを確保しておく
	msg=kmalloc(GFP_KERNEL,10*sizeof(char));
	
	count_process();
	return 0;
}

void proc_cleanup(void) {
	remove_proc_entry("hello",NULL);
}

MODULE_LICENSE("GPL"); 
module_init(proc_init);
module_exit(proc_cleanup);

