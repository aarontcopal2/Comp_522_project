//******************************************************************************
// system includes
//******************************************************************************

#include <time.h>       // clock, clock_t
#include <pthread.h>    // pthread_self, pthread_t, pthread_create, pthread_join
#include <stdlib.h>     // malloc



//******************************************************************************
// local includes
//******************************************************************************

#include "../hashtable/splay-tree/splay-uint64.h"
#include "benchmark.h"



//******************************************************************************
// splay-tree definitions
//******************************************************************************

#define st_insert				\
  typed_splay_insert(int)

#define st_lookup				\
  typed_splay_lookup(int)

#define st_delete				\
  typed_splay_delete(int)

#define st_forall				\
  typed_splay_forall(int)

#define st_count				\
  typed_splay_count(int)


// we have a special case where we only need to search for keys,
// we are not concerned with the value. Can we remove the value parameter from the struct?
typedef struct typed_splay_node(int) {
  struct typed_splay_node(int) *left;
  struct typed_splay_node(int) *right;
  uint64_t key;
  address *val;
} typed_splay_node(int);


typedef typed_splay_node(int) splay_t;


typed_splay_impl(int)



//******************************************************************************
// local data
//******************************************************************************

splay_t *root = 0;

static address addrs[123] = {
{"716 Glenlake Ave. "},
{"Powhatan, VA 23139"},
{"9628 Middle River Street "},
{"Grand Forks, ND 58201"},
{"8512 S. Arnold Drive "},
{"Scarsdale, NY 10583"},
{"7521 Walnut Ave."}, 
{"Port Jefferson Station, NY 11776"},
{"26 Tanglewood Dr. "},
{"Tallahassee, FL 32303"},
{"331 West Lexington Street "},
{"Oshkosh, WI 54901"},
{"59 Cherry Road "},
{"Massapequa, NY 11758"},
{"7930 Nichols St."}, 
{"Mableton, GA 30126"},
{"609 Delaware Road "},
{"Unit 905 "},
{"Nicholasville, KY 40356"},
{"35 Bowman Drive "},
{"Apt 2 "},
{"Livingston, NJ 07039"},
{"21 Sycamore Lane "},
{"Carpentersville, IL 60110"},
{"84 Forest Dr. "},
{"Windermere, FL 34786"},
{"8044 Goldfield St. "},
{"Newtown, PA 18940"},
{"248 S. East St. "},
{"Huntington, NY 11743"},
{"85 Arnold Street "},
{"Apt E "},
{"Central Islip, NY 11722"},
{"7468 Maiden St. "},
{"Thomasville, NC 27360"},
{"51 Devon Avenue "},
{"Palm Harbor, FL 34683"},
{"238 N. Orange Dr. "},
{"Minot, ND 58701"},
{"39 Big Rock Cove Lane "},
{"Plainfield, NJ 07060"},
{"99 Maple Road "},
{"Pearl, MS 39208"},
{"147 East Del Monte Rd. "},
{"Attleboro, MA 02703"},
{"667 Saxton Road "},
{"Sumter, SC 29150"},
{"8195 E. Roehampton St. "},
{"Lincolnton, NC 28092"},
{"34 E. Oak Dr. "},
{"Blackwood, NJ 08012"},
{"287 Vernon Ave. "},
{"Reisterstown, MD 21136"},
{"957 Temple Avenue "},
{"Bowie, MD 20715"},
{"766 Buckingham St. "},
{"Hamilton, OH 45011"},
{"907 Pin Oak Dr. "},
{"Rolla, MO 65401"},
{"8617 Beech Street "},
{"Jacksonville, NC 28540"},
{"39 South Tower St. "},
{"Grove City, OH 43123"},
{"54 Pumpkin Hill Court "},
{"Elizabethton, TN 37643"},
{"47 Fieldstone St. "},
{"Wilmette, IL 60091"},
{"424 Crescent Drive "},
{"Parkersburg, WV 26101"},
{"7672 N. Stonybrook Ave. "},
{"Port Huron, MI 48060"},
{"40 Morris Lane "},
{"Flushing, NY 11354"},
{"19 10th Ave. "},
{"Burbank, IL 60459"},
{"7 Hartford St. "},
{"Norristown, PA 19401"},
{"65 Manhattan Street "},
{"Coram, NY 11727"},
{"46 Arcadia Street "},
{"Clarksburg, WV 26301"},
{"533 Glen Ridge St. "},
{"Prattville, AL 36067"},
{"64 Blue Spring St. "},
{"Neenah, WI 54956,"},
{"3 Mulberry St. "},
{"Elk Grove Village, IL 60007"},
{"44 Cedar Swamp Ave. "},
{"Loveland, OH 45140"},
{"86 Chapel Drive "},
{"West Roxbury, MA 02132"},
{"8154 Briarwood Street "},
{"Tuckerton, NJ 08087"},
{"161 Hudson St."},
{"Newton, NJ 07860"},
{"93 Cobblestone Dr. "},
{"Coachella, CA 92236"},
{"8480 Pawnee Ave."}, 
{"Trussville, AL 35173"},
{"2 N. Big Rock Cove Ave. "},
{"Moses Lake, WA 98837"},
{"7727 Rockcrest St. "},
{"Onalaska, WI 54650"},
{"7 W. Euclid Street "},
{"Cary, NC 27511"},
{"8537 Market Rd."},
{"Gibsonia, PA 15044"},
{"93 East Beechwood Street "},
{"Gallatin, TN 37066"},
{"79 Fairway Rd."},
{"Evanston, IL 60201"},
{"819 Marlborough Road "},
{"East Hartford, CT 06118"},
{"52 South Sleepy Hollow Street "},
{"Fort Lauderdale, FL 33308"},
{"88 Rock Maple St. "},
{"State College, PA 16801"},
{"201 Bear Hill St. "},
{"Elizabeth, NJ 07202"},
{"9198 Griffin Ave. "},
{"Southgate, MI 48195"},
{"14 Lakewood Lane"},
{"Mchenry, IL 60050"}};



//******************************************************************************
// private operations
//******************************************************************************

static splay_t* splay_node(uint64_t key, address *val) {
  splay_t *node = (splay_t *) malloc(sizeof(splay_t));
  node->left = node->right = NULL;
  node->key = key;
  node->val = val;
  return node;
}


static void initialize_splay_tree() {
    // initializing the splay tree with 1k elements
    for (int i = 0; i < 1000; i++) {
        uint64_t key = i;
        address *val = (void*)&addrs[key];
        splay_t *node = splay_node(key, val);
        st_insert(&root, node);
        free(node);
    }
}


static void *splay_tree_thread_operations(void *arg) {
    bool status;
    uint t_index = pthread_self();
    int random_start_index = t_index % (ADDRESS_SIZE-20);

    for (int i = 0; i < 20; i++) {
        uint64_t key = random_start_index + i;
        address *val = (void*)&addrs[key];

        splay_t *node = splay_node(key, val);
        st_insert(&root, node);
        free(node);

        address *result_val = (address*) st_lookup(&root, key);

        status = st_delete(&root, key);

        result_val = (address*) st_lookup(&root, key);
    }
}



//******************************************************************************
// interface operations
//******************************************************************************

void splay_tree_benchmark () {
  initialize_splay_tree();
     
  clock_t start = clock();
    
  int THREADS = 30;
  pthread_t thr[THREADS];
  int *index = malloc(sizeof(int) * THREADS);

  for (int i = 0; i < THREADS; i++) {
      index[i] = i;
      pthread_create(&thr[i], NULL, splay_tree_thread_operations, &index[i]);
  }
  for (int i = 0; i < THREADS; i++) {
      pthread_join(thr[i], NULL);
  }

  free(index);
    
  clock_t end = clock();
  double cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
}
