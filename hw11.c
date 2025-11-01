/*
 * Homework 11
 * Zubin Sidhu
 * CS 240, Spring 2025
 * Purdue University
 */

#include "hw11.h"

#include <assert.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>

/*
 * Create Ring - Allocates and initializes a new ring_t structure representing
 * a web ring
 */

ring_t *create_ring(char *ring_name) {
  assert(ring_name != NULL);

  ring_t *new_ring = malloc(sizeof(ring_t));
  assert(new_ring != NULL);

  char *copy = malloc(strlen(ring_name) + 1);
  assert(copy != NULL);
  strcpy(copy, ring_name);

  new_ring->ring_name = copy;
  new_ring->site_list = NULL;
  new_ring->left = NULL;
  new_ring->right = NULL;

  return new_ring;
} /* create_ring() */

/*
 * Add Site to Ring - Allocates and inserts a new site_t node into the given
 * ring's site_list. The sitesin the ring are ordered first by increasing
 * length of their names. If two site names have equal length, they are ordered
 * lexicographically. The new site is inserted into the ring at the position
 * that maintains this ordering. The site_list pointers of the ring is then set
 * to the new head of the CDLL. If the ring has no sites, the previous and next
 * pointers of this site point to itself.
 */

void add_site_to_ring(ring_t *ring, char *site_name) {
  assert(ring != NULL);
  assert(site_name != NULL);

  site_t *new_site = malloc(sizeof(site_t));
  assert(new_site != NULL);

  char *copy = malloc(strlen(site_name) + 1);
  assert(copy != NULL);
  strcpy(copy, site_name);

  new_site->site_name = copy;
  new_site->next_site = NULL;
  new_site->prev_site = NULL;

  /*
   * If the site_list is empty, set this site as the head and have its next and
   * previous pointer set to itself
   */

  if (ring->site_list == NULL) {
    new_site->next_site = new_site;
    new_site->prev_site = new_site;
    ring->site_list = new_site;
    return;
  }

  /* Find the head of the circular doubly linked list */

  site_t *head = ring->site_list;
  do {
    if (strlen(head->prev_site->site_name) < strlen(head->site_name)) {
      head = head->prev_site;
    }
    else if (strlen(head->prev_site->site_name) == strlen(head->site_name)) {
      if (strcmp(head->prev_site->site_name, head->site_name) < 0) {
        head = head->prev_site;
      }
      else {
        break;
      }
    }
    else {
      break;
    }
  } while (head != ring->site_list);

  /*
   * Starting with the head, loop through each site and insert by ascending
   * length, if two have the same length, sort by lexicographical order
   */

  site_t *curr = head;
  do {
    if (strlen(curr->site_name) < strlen(copy)) {
      curr = curr->next_site;
    }
    else if (strlen(curr->site_name) == strlen(copy)) {
      if (strcmp(curr->site_name, copy) < 0) {
        curr = curr->next_site;
      }
      else if (strcmp(curr->site_name, copy) == 0) {

        /* Found a duplicate site, don't insert */

        return;
      }
      else {
        break;
      }
    }
    else {
      break;
    }
  } while (curr != head);

  /* Insert the new site before curr */

  new_site->next_site = curr;
  new_site->prev_site = curr->prev_site;
  curr->prev_site->next_site = new_site;
  curr->prev_site = new_site;

  /* Check for head pointer adjustment */

  if (strlen(head->site_name) > strlen(new_site->site_name)) {
    head = new_site;
  }
  else if (strlen(head->site_name) == strlen(new_site->site_name)) {
    if (strcmp(head->site_name, new_site->site_name) > 0) {
      head = new_site;
    }
  }
  ring->site_list = head;
} /* add_site_to_ring() */

/*
 * Insert Ring - Inserts the new_ring into the binary search tree rooted at
 * root based on lexicographical order of ring_name. If the tree is empty, the
 * root is set to new_ring
 */

void insert_ring(ring_t **root, ring_t *new_ring) {
  assert(root != NULL);
  assert(new_ring != NULL);

  if (*root == NULL) {
    *root = new_ring;
    return;
  }

  if (strcmp(new_ring->ring_name, (*root)->ring_name) < 0) {
    insert_ring(&((*root)->left), new_ring);
  }
  else {
    insert_ring(&((*root)->right), new_ring);
  }
} /* insert_ring() */

/*
 * Count Sites - Helper function that goes through the list of sites in
 * a site list and counts the number of sites in the list
 */

int count_sites(site_t *head) {
  if (head == NULL) {
    return 0;
  }
  int count = 0;
  site_t *curr = head;
  do {
    count++;
    curr = curr->next_site;
  } while (curr != head);
  return count;
} /* count_sites() */

/*
 * Free CDLL - Helper function that given a starting site, this function loops
 * through all the sites in a site list and frees all the sites looping back to
 * the head and then freeing the starting site as well
 */

void free_cdll(site_t *head) {
  if (head == NULL) {
    return;
  }

  int num_sites = count_sites(head);
  site_t *curr = head;

  while (num_sites > 0) {
    site_t *temp = curr;

    curr->prev_site->next_site = curr->next_site;
    curr->next_site->prev_site = curr->prev_site;
    curr = curr->next_site;

    free(temp->site_name);
    free(temp);

    num_sites--;
  }
} /* free_cdll() */

/*
 * Purge Small Rings - Traverses the binary search tree of rings and removes
 * any rings with fewer than the parameter min_sites. Uses free_cdll to
 * deallocate all the CDLL sites and removes the ring node while maintaining
 * the binary search tree integrity. Returns the number of purged rings
 */

int purge_small_rings(ring_t **root, int min_sites) {
  assert(root != NULL);
  assert(min_sites >= 0);
  if (*root == NULL) {
    return 0;
  }

  int count = 0;
  if ((*root)->left != NULL) {
    count += purge_small_rings(&((*root)->left), min_sites);
  }
  if ((*root)->right != NULL) {
    count += purge_small_rings(&((*root)->right), min_sites);
  }

  int site_count = count_sites((*root)->site_list);
  if (site_count >= min_sites) {
    return count;
  }

  free_cdll((*root)->site_list);
  count++;

  /* Free the ring itself */

  ring_t *temp = NULL;

  /* If ring is a leaf node just free it */

  if (((*root)->left == NULL) && ((*root)->right == NULL)) {
    free((*root)->ring_name);
    free(*root);
    *root = NULL;
  }

  /* Ring with only one child */

  else if ((((*root)->left == NULL) && ((*root)->right != NULL)) ||
           ((((*root)->left != NULL) && ((*root)->right == NULL)))) {
    temp = (*root)->left ? (*root)->left : (*root)->right;
    free((*root)->ring_name);
    free(*root);
    *root = temp;
  }

  /* Ring with two children */

  else {

    /* Find minimum node on right child */

    //printf("DEBUG::Double Child Node; root = %s, root right = %s\n", (*root)->ring_name, (*root)->right->ring_name);

    ring_t *parent = *root;
    ring_t *node = (*root)->right;
    while ((node != NULL) && (node->left != NULL)) {
      parent = node;
      node = node->left;

      //printf("DEBUG::Traversing right tree; parent = %s, node = %s\n", parent->ring_name, node->ring_name);
    }

    char *copy = malloc(strlen(node->ring_name) + 1);
    assert(copy != NULL);
    strcpy(copy, node->ring_name);
    free((*root)->ring_name);
    (*root)->ring_name = copy;

    //printf("DEBUG::Successfully replaced root name, %s\n", (*root)->ring_name);

    (*root)->site_list = node->site_list;

    //printf("DEBUG::Swapped site list of root witu node's\n");

    if (parent != *root) {
      parent->left = node->right;
    }
    else {
      parent->right = node->right;
    }

    //printf("DEBUG::Updated the parent's pointer to the child node's right children\n");

    free(node->ring_name);
    node->ring_name = NULL;
    free(node);
    node = NULL;

    //printf("DEBUG::Deleted old child node\n");
  }

  return count;
} /* purge_small_rings() */

/*
 * Delete Dead Links - Searches through the binary search tree rooted at the
 * parameter root and scans each ring's site list for the site_name containing
 * the flag word and removes those sites. After each modification, the ring's
 * site list points to the head of the CDLL and the function returns the total
 * number of sites removed from all rings
 */

int delete_dead_links(ring_t *root, char *flag_word) {
  assert(root != NULL);
  assert(flag_word != NULL);

  int count = 0;

  //printf("DEBUG::Processing Ring %s\n", root->ring_name);

  if (root->left != NULL) {
    count += delete_dead_links(root->left, flag_word);
    //printf("DEBUG::Deleted sites on left side: Count = %d\n", count);
  }
  if (root->right != NULL) {
    count += delete_dead_links(root->right, flag_word);
    //printf("DEBUG::Deleted sites on right side: Count = %d\n", count);
  }

  site_t *curr = root->site_list;
  if (curr == NULL) {
    //printf("End of List: Count = %d\n", count);
    return count;
  }

  int num_sites = count_sites(root->site_list);
  do {
/*
    if (curr == NULL || root->site_list == NULL) {
      printf("DEBUG::Curr is null or root is null, count = %d\n", count);
      break;
    }
    printf("DEBUG::Processing site %s, head = %s, flag = %s\n",
            curr->site_name, root->site_list->site_name, flag_word);
*/
    site_t *next = curr->next_site;
    if (strstr(curr->site_name, flag_word) != NULL) {

      //printf("DEBUG::Found flag word\n");

      /* Check if only one node */

      if (curr->next_site == curr) {
        //printf("DEBUG::Only one site in list and matches flag word\n");
        count++;
        free(curr->site_name);
        curr->site_name = NULL;
        free(curr);
        curr = NULL;
        root->site_list = NULL;
        //printf("DEBUG::Count = %d\n", count);
        return count;
      }

      curr->prev_site->next_site = curr->next_site;
      curr->next_site->prev_site = curr->prev_site;
      curr->next_site = NULL;
      curr->prev_site = NULL;

      if (curr == root->site_list) {
        root->site_list = next;
      }

      free(curr->site_name);
      curr->site_name = NULL;
      free(curr);
      curr = NULL;
      count++;

      //printf("DEBUG::Deleted Site and Incremented Count. Count = %d\n", count);
    }
    curr = next;
    num_sites--;
  } while (num_sites > 0);

  return count;
} /* delete_dead_links() */

/*
 * Total Surf Chain Length - Computes the total number of sites across all
 * rings in the binary search tree. It traverses the binary search tree, counts
 * the sites in each ring, and returns the total length of the surfing session
 */

int total_surf_chain_length(ring_t *root) {
  assert(root != NULL);
  if (root == NULL) {
    return 0;
  }

  int count = 0;
  count += count_sites(root->site_list);

  if (root->left != NULL) {
    count += total_surf_chain_length(root->left);
  }
  if (root->right != NULL) {
    count += total_surf_chain_length(root->right);
  }

  return count;
} /* total_surf_chain_length() */

/*
 * Scavenge Neighboring Rings - Traverses the binary search tree rooted at root
 * and identifies the rings with the fewer than min_sites_needed parameter. For
 * each underpopulated ring, the function takes one of its child ring's sites
 * to reach min_sites
 */

int scavenge_neighboring_rings(ring_t *root, int min_sites) {
  assert(root != NULL);
  assert(min_sites >= 0);
  if (root == NULL) {
    return 0;
  }
  //printf("DEBUG::Checking Ring Node %s, Min Sites = %d, Node Sites = %d\n", root->ring_name, min_sites, count_sites(root->site_list));

  int total = 0;
  if (root->left != NULL) {
    total += scavenge_neighboring_rings(root->left, min_sites);
  }
  if (root->right != NULL) {
    total += scavenge_neighboring_rings(root->right, min_sites);
  }

  /* This ring has enough sites so return the total scavenged from its
   * children */

  int site_count = count_sites(root->site_list);
  if (site_count >= min_sites) {
    //printf("DEBUG::Count = %d\n", total);
    return total;
  }

  /* Pick the child ring with the greater amount of sites to scavenge from */

  ring_t *left = root->left;
  ring_t *right = root->right;
  ring_t *source = NULL;

  int left_count = left ? count_sites(left->site_list) : 0;
  int right_count = right ? count_sites(right->site_list) : 0;

  if ((left_count > min_sites) && (left_count >= right_count)) {
    source = left;
  }
  else if ((right_count > min_sites) && (right_count >= left_count)) {
    source = right;
  }
  else {
    //printf("DEBUG::Count = %d\n", total);
    return total;
  }

  /* Scavenge sites from the chosen child ring by picking its smallest length
   * site names */

  int needed = min_sites - site_count;

  /* Ensure that the chosen child has at least needed sites in excess of
   * min_sites so that removal will not drop it below min_sites */

  if ((count_sites(source->site_list) - min_sites) < needed) {
    return total;
  }

  int scavenged = 0;

  while ((needed > 0) && (source->site_list != NULL) &&
         (count_sites(source->site_list) > min_sites)) {
    site_t *smallest_site = source->site_list;
    site_t *curr = source->site_list->next_site;
    int num_sites = count_sites(source->site_list);
    do {
      if ((strlen(curr->site_name) < strlen(smallest_site->site_name)) ||
          (strlen(curr->site_name) == strlen(smallest_site->site_name) &&
           strcmp(curr->site_name, smallest_site->site_name) < 0)) {
        smallest_site = curr;
      }
      curr = curr->next_site;
      num_sites--;
    } while (num_sites > 0);

    //printf("Min Sites = %d\n", min_sites);
    /* printf("DEBUG::Scavenging from Node %s (%d) to parent node %s (%d)\n",
     *       source->ring_name, count_sites(source->site_list), root->ring_name,
     *       count_sites(root->site_list)); */

    if (smallest_site->next_site == smallest_site) {
      //printf("DEBUG::Smallest site is the only node!!!!!!!!!!\n");
      source->site_list = NULL;
    }
    else {
      smallest_site->prev_site->next_site = smallest_site->next_site;
      smallest_site->next_site->prev_site = smallest_site->prev_site;
      if (source->site_list == smallest_site) {
        source->site_list = smallest_site->next_site;
      }
    }

    add_site_to_ring(root, smallest_site->site_name);
    free(smallest_site->site_name);
    free(smallest_site);
    needed--;
    scavenged++;
  }
  total += scavenged;
  //printf("DEBUG::Count = %d\n", total);
  return total;
} /* scavenge_neighboring_rings() */

/*
 * Delete Network - Loops through and deallocates all the entries in the binary
 * search tree web ring network and sets the root to NULL
 */

void delete_network(ring_t **root) {
  assert(root != NULL);
  if (*root == NULL) {
    return;
  }

  delete_network(&((*root)->left));
  delete_network(&((*root)->right));
  free_cdll((*root)->site_list);
  free((*root)->ring_name);
  free(*root);
  *root = NULL;
} /* delete_network() */
