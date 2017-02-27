#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include <linux/slab.h>
#include "../kernel/sched/sched.h"
#include <linux/rbtree.h>
#include <linux/string.h>

#define PRIO_CNT 40

int len,temp;
char prio[PRIO_CNT];
char *msg;

char *make_str(char *pt, char *str)
{
	strcat(pt, str);
	pt += sizeof(str);
	return pt;
}

// ランキューから、赤黒木のルートノードを取得
struct rb_node *get_root(void)
{
	int cpu = smp_processor_id();
	struct rq *rq = cpu_rq(cpu);
	struct rb_node *root = (rq->cfs).tasks_timeline.rb_node;
	return root;
}

// ノードに対応したプロセスの優先度取得
void get_priority(struct rb_node *node)
{
	
	struct sched_entity *s_entity;
	struct task_struct *task;

	s_entity = rb_entry(node, struct sched_entity, run_node);	// sched_entry構造体取得
	task = container_of(s_entity, struct task_struct, se);		// task構造体取得
	prio[task->static_prio - 100]++;							// 該当優先度のプロセス数をカウント
	printk( KERN_INFO "task_priority:%d", task->static_prio );
}

// 二分木走査
void rb_scan(struct rb_node *node)
{
	get_priority(node);
	rb_scan(node->rb_left);
	rb_scan(node->rb_right);
}

// msg にデータを書き出す
// itoa() は使用できず、代替方法を探す必要あり
void format_data(void)
{
	int i;
	char *p;

	make_str(msg, "priority\t\tcount\n");
	for(i = 0; i < PRIO_CNT; i++){
		if(prio[i])
			make_str(p, itoa(i, NULL, DECIMAL));
			p = make_str(p, "\t\t");
			make_str(p, itoa(prio[i], NULL, DECIMAL));
		}
	}
}

// 処理の取りまとめ関数
void count_process(void)
{
	struct rb_node *root = get_root();
	rb_scan(root);
}

// cat /proc/miracle 時に呼ばれる
ssize_t read_proc (struct file *fp, char __user *buf, size_t count, loff_t *ppos, int is_user)
{
	count_process();
	format_data();
	copy_to_user(buf, msg, strlen(msg));	// msg から プロセス空間の buf にデータコピー
	return strlen(msg);
}

// モジュールAPIの登録
struct file_operations proc_fops = {
	owner:THIS_MODULE,
	read: read_proc,
	//write: write_proc
};

// 初期化関数
int proc_init (void) 
{
	proc_create("miracle",0666,NULL,&proc_fops);	// /proc/sys/kernel 配下へ作成できず
	msg=kmalloc(GFP_KERNEL,500*sizeof(char));		// ユーザ空間に渡すデータのバッファ
	
	return 0;
}

void proc_cleanup(void) {
	remove_proc_entry("miracle",NULL);
}

MODULE_LICENSE("GPL"); 
module_init(proc_init);
module_exit(proc_cleanup);

