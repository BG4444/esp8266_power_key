#ifndef LINKED_LIST_H
#define LINKED_LIST_H


#define dADD_ITEM(type)     type *add_##type##_item(type **current)
#define dDELETE_ITEM(type)  void delete_##type##_item(type **current, const type *item)

#define ADD_ITEM(type)     dADD_ITEM(type)\
                           {\
                                for(;*current;current=& (*current)->next);\
                                *current=(type*)os_malloc(sizeof(type));\
                                (*current)->next=0;\
                                return *current;\
                           }

#define DELETE_ITEM(type)  dDELETE_ITEM(type)\
                           {\
                                for(;*current!=item;current=& (*current)->next);\
                                *current=(*current)->next;\
                                os_free(item);\
                           }

#endif // LINKED_LIST_H
