#include <stdio.h>
#include <assert.h>

int main() {
  FILE *fp = fopen("/share/files/num", "r+");
  printf("here1\n");
  assert(fp);

  fseek(fp, 0, SEEK_END);
  long size = ftell(fp);
  printf("here2\n");
  assert(size == 5000);

  fseek(fp, 500 * 5, SEEK_SET);
  int i, n;
  for (i = 500; i < 1000; i ++) {
    fscanf(fp, "%d", &n);
    printf("here3\n");
    assert(n == i + 1);
  }

  fseek(fp, 0, SEEK_SET);
  for (i = 0; i < 500; i ++) {
    fprintf(fp, "%4d\n", i + 1 + 1000);
  }

  for (i = 500; i < 1000; i ++) {
    fscanf(fp, "%d", &n);
    printf("here4\n");
    assert(n == i + 1);
  }

  fseek(fp, 0, SEEK_SET);
  for (i = 0; i < 500; i ++) {
    fscanf(fp, "%d", &n);
    printf("here5\n");
    assert(n == i + 1 + 1000);
  }

  fclose(fp);

  printf("PASS!!!\n");

  return 0;
}
