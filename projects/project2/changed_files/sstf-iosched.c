/*
 * elevator sstf
 */
#include <linux/blkdev.h>
#include <linux/elevator.h>
#include <linux/bio.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>

struct sstf_data {
	struct list_head queue;
	struct list_head *dude;
	int size;
};

static void sstf_merged_requests(struct request_queue *q, struct request *rq,
				 struct request *next)
{
	list_del_init(&next->queuelist);
}

static int sstf_dispatch(struct request_queue *q, int force)
{
	struct sstf_data *sd = q->elevator->elevator_data;

	// variables for getting access to request objects when iterating over the queue
	struct request *rq, *req, *req_next, *req_prev;

	// sector values are to indicate sector location so we can see whether
	// to move forward or back in the queue. For calculating
	// the distance to next or previous to make that decision.
	unsigned long curr_sector, next_sector, prev_sector;

	printk( "SSTF: GOT INTO DISP\n");

	if (!list_empty(&sd->queue)) {
		rq = list_entry(sd->dude, struct request, queuelist);

		if (sd->size == 1) {
			printk( "SSTF: queue w/ one item\n");
			list_del_init(&rq->queuelist);
			sd->size -= 1;
		} else {
			// get request information for dude request and neighboring
			req = list_entry(sd->dude, struct request, queuelist);
                	req_next = list_entry(sd->dude->next, struct request, queuelist);
			req_prev = list_entry(sd->dude->prev, struct request, queuelist);

			// get sector positions for each request
			curr_sector = (unsigned long) blk_rq_pos(req);
			next_sector = (unsigned long) blk_rq_pos(req_next);
			prev_sector = (unsigned long) blk_rq_pos(req_prev);

			if (abs(curr_sector - prev_sector) < abs(curr_sector - next_sector)) {
				printk( "SSTF: setting next request for dispatch to prev\n");

				if (prev_sector != 0) {
                                        printk("SSTF LESSER 1\n");
                                        sd->dude = sd->dude->prev;
                                } else if (next_sector != 0) {
                                        printk("SSTF LESSER 2\n");
                                        sd->dude = sd->dude->next;
                                } else {
                                        printk("SSTF LESSER 3\n");
					sd->dude = sd->queue.next;
                                }

				/*
				if (!list_empty(sd->dude->prev)) {
					printk("SSTF LESSER 1\n");
					sd->dude = sd->dude->prev;
				} else if (!list_empty(sd->dude->next)) {
					printk("SSTF LESSER 2\n");
					sd->dude = sd->dude->next;
				} else {
					printk("SSTF LESSER 3\n");
					return 0;
				}
				*/
			} else {
				printk( "SSTF: setting next request for dispatch to next\n");

				if (next_sector !=0 ) {      
                                        printk("SSTF GREATER 1\n");
                                        sd->dude = sd->dude->next;
                                } else if (prev_sector !=0 ) {
                                        printk("SSTF GREATER 2\n");
                                        sd->dude = sd->dude->prev;
                                } else {
                                        printk("SSTF GREATER 3\n");
					sd->dude = sd->queue.next;
                                }

				/*
				if (!list_empty(sd->dude->next)) {					
					printk("SSTF GREATER 1\n");
					sd->dude = sd->dude->next;
				} else if (!list_empty(sd->dude->prev)) {
					printk("SSTF GREATER 2\n");
					sd->dude = sd->dude->prev;
				} else {
					printk("SSTF GREATER 3\n");
					return 0;
				}
				*/
			}
						
			list_del_init(&rq->queuelist);
			sd->size -= 1;
		}
		
		printk( "SSTF: +DISPATCHING+\n");
		printk("dispatching request: %lu\n", (unsigned long)blk_rq_pos(rq));
		elv_dispatch_sort(q, rq);
		printk( "SSTF: -DISPATCHED-\n");
		return 1;
	}
	return 0;
}

static void sstf_add_request(struct request_queue *q, struct request *rq)
{
	struct sstf_data *sd = q->elevator->elevator_data;
	struct list_head *temp;

	printk( "SSTF: GOT INTO ADD\n");

	// variables for getting access to request objects when iterating over the queue
	struct request *req, *req_next;

	// temporary variables for comparing sector positions
	sector_t curr_sector, next_sector, req_sector;

	int merged = 0;
	
	// first, check if the list is empty because our job is easy.
	if (list_empty(&sd->queue)) {
		printk( "SSTF: adding to empty queue.\n");
		list_add(&rq->queuelist, &sd->queue);
		sd->dude = sd->queue.next;
		sd->size += 1;
		return;
	}

	list_for_each(temp, &sd->queue) {
		if (sd->size == 1) {
			printk( "SSTF: adding to queue w/ 1 item.\n");
			list_add(&rq->queuelist, temp);
			sd->size += 1;
			merged = 1;
			break;
		}

		// get request information for current list item and next
		req = list_entry(temp, struct request, queuelist);
		req_next = list_entry(temp->next, struct request, queuelist);

		// get our sector data for this request, the next, and the main request
		curr_sector = blk_rq_pos(req);
		next_sector = blk_rq_pos(req_next);
		req_sector = blk_rq_pos(rq);

		if (curr_sector <= req_sector && next_sector >= req_sector) {
			printk( "SSTF: normal add to queue.\n");
			list_add_tail(&rq->queuelist, temp);
			sd->size += 1;
			merged = 1;
			break;
		}
	}

	if (merged == 0) {
		printk( "SSTF: couldn't find a spot, adding to end\n");
		list_add_tail(&rq->queuelist, &sd->queue);
		sd->size += 1;
	}
}

static struct request * sstf_former_request(struct request_queue *q, struct request *rq)
{
	struct sstf_data *nd = q->elevator->elevator_data;

	if (rq->queuelist.prev == &nd->queue)
		return NULL;
	return list_entry(rq->queuelist.prev, struct request, queuelist);
}

static struct request * sstf_latter_request(struct request_queue *q, struct request *rq)
{
	struct sstf_data *nd = q->elevator->elevator_data;

	if (rq->queuelist.next == &nd->queue)
		return NULL;
	return list_entry(rq->queuelist.next, struct request, queuelist);
}

static int sstf_init_queue(struct request_queue *q, struct elevator_type *e)
{
	struct sstf_data *nd;
	struct elevator_queue *eq;

	eq = elevator_alloc(q, e);
	if (!eq)
		return -ENOMEM;

	nd = kmalloc_node(sizeof(*nd), GFP_KERNEL, q->node);
	if (!nd) {
		kobject_put(&eq->kobj);
		return -ENOMEM;
	}
	eq->elevator_data = nd;

	INIT_LIST_HEAD(&nd->queue);

	spin_lock_irq(q->queue_lock);
	q->elevator = eq;
	spin_unlock_irq(q->queue_lock);
	return 0;
}

static void sstf_exit_queue(struct elevator_queue *e)
{
	struct sstf_data *nd = e->elevator_data;

	BUG_ON(!list_empty(&nd->queue));
	kfree(nd);
}

static struct elevator_type elevator_sstf = {
	.ops = {
		.elevator_merge_req_fn		= sstf_merged_requests,
		.elevator_dispatch_fn		= sstf_dispatch,
		.elevator_add_req_fn		= sstf_add_request,
		.elevator_former_req_fn		= sstf_former_request,
		.elevator_latter_req_fn		= sstf_latter_request,
		.elevator_init_fn		= sstf_init_queue,
		.elevator_exit_fn		= sstf_exit_queue,
	},
	.elevator_name = "sstf",
	.elevator_owner = THIS_MODULE,
};

static int __init sstf_init(void)
{
	return elv_register(&elevator_sstf);
}

static void __exit sstf_exit(void)
{
	elv_unregister(&elevator_sstf);
}

module_init(sstf_init);
module_exit(sstf_exit);


MODULE_AUTHOR("Ty Skelton");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("SSTF IO scheduler");
