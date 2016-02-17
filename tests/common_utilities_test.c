#include "../include/common_utilities.h"

int main(void){
   //check the is_prefix function
   printf("%d\n", is_prefix("ab", "a"));
   printf("%d\n", is_prefix("abddd", "ab"));
   printf("%d\n", is_prefix("sab", "a"));
   printf("%d\n", is_prefix("a", "ad"));
   return 0;
}
