#include "clonk.h"

void merge(uint16_t *arr, uint16_t l, uint16_t m, uint16_t r)
{
    uint16_t i, j, k;
    uint16_t n1 = m - l + 1;
    int n2 =  r - m;

    // Create temp arr
    uint16_t L[n1], R[n2];

    // Copy data to temp arrays L[] and R[]
    for (i = 0; i < n1; i++)
        L[i] = arr[l + i];
    for (j = 0; j < n2; j++)
        R[j] = arr[m + 1+ j];

     // Merge the temp arrays back into arr[l..r]

     i = 0; // Initial index of first subarray
     j = 0; // Initial index of second subarray
     k = l; // Initial index of merged subarray

     while (i < n1 && j < n2)
     {
         if (L[i] <= R[j])
         {
             arr[k] = L[i];
             i++;
         }
         else
         {
             arr[k] = R[j];
             j++;
         }
         k++;
     }

     // Copy the remaining elements of L[], if there are any
     while (i < n1)
     {
         arr[k] = L[i];
         i++;
         k++;
     }

     // Copy the remaining elements of R[], if there are any
     while (j < n2)
     {
         arr[k] = R[j];
         j++;
         k++;
     }
 }

void onk_merge_sort_u16(uint16_t *arr, uint16_t l, uint16_t r)
{
    if (l < r)
    {
        // Same as (l+r)/2, but avoids overflow for large l and h
        int m = l+(r-l)/2;

        // Sort first and second halves
        onk_merge_sort_u16(arr, l, m);
        onk_merge_sort_u16(arr, m+1, r);

        merge(arr, l, m, r);
     }
 }
