#include <iostream>
#include <string>
#include <cstring>
#include <set>
#include <fstream>
#include <vector>
#include <cmath>
#include <unistd.h>
#include <assert.h>
#include <semaphore.h>
#include <pthread.h>
using namespace std;
#define length 500 //∆•≈‰≥§∂»
#define next_match_interval (1512*30)
#define BUFSIZE 30240000
#define POPBUFLEN 1512000
#define start_len 1400
#define frame_len 1512
#define RESYNC_LEN (1512*126)
#define ones_threshold 0.02
#define min_diff 302
#define match_limit 1400
#define min_match_distance 604800
#define cyc 63

extern "C" {
char buf1[BUFSIZE];
char buf2[BUFSIZE];
int buf1_index = 0, buf2_index = 0, cur_decode_index = 0;
int prevmatch = 0;
sem_t buf1_mutex, buf1_ready, buf2_ready, buf2_mutex;

int different_match = 0;
int KMPStrMatching(const char *T, const char *P, const int *N, int t, int p) {
    int i = 0;
    int j = 0;
    int pLen = p;
    int tLen = t;

    if (tLen < pLen)
        return (-1);
    while (i < pLen && j < tLen) {
        if (i == -1 || T[j] == P[i]) {
            i++;
            j++;
        } else i = N[i];
    }
    if (i >= pLen)
        return (j - pLen + 1);
    return (-1);
}
int compute_sim(const char *a, const char *b) {
    int cnt = 0;
    for (int i = 0; i < frame_len; i++) {
        if (a[i] == b[i]) {
            cnt++;
        }
    }
    return cnt;
}

int cntones(const char *buf, int len) {
    int cnt = 0;
    for (int i = 0; i < len; ++i) {
        if (buf[i] == 1) {
            cnt++;
        }
    }
    return cnt;
}

int badcnt(const string &s) {
    int cnt = 0;
    for (int i = 0; i < s.size(); i += 3) {
        if (s.substr(i, 3) == "001" || s.substr(i, 3) == "010" || s.substr(i, 3) == "100") {
            cnt++;
        }
    }
    return cnt;
}

int find_first_match(const char *buf1, const char *buf2, int framecnt, int buf2len, int maxdistance = min_match_distance) {
    int slotcnt = min(buf2len - framecnt * frame_len, prevmatch + maxdistance);
    for (int i = prevmatch; i < slotcnt; i += frame_len) {
        int symbol_match_cnt = 0;
        for (int j = 0; j < framecnt * frame_len; j += frame_len) {
            int sim = compute_sim(buf1 + j, buf2 + i + j);
            if (sim > match_limit)
                symbol_match_cnt++;
        }
        if (symbol_match_cnt >= 4) {
            prevmatch = i;
            return i;
        }
    }
    return -1;
}

int *findNext(const char *P, int p) {
    int i = 0;
    int k = -1;
    int m = p;
    assert(m > 0);
    int *next = new int[m];
    assert(next != 0);
    next[0] = -1;
    while (i < m) {
        while (k >= 0 && P[i] != P[k])
            k = next[k];
        i++;
        k++;
        if (i == m) break;
        if (P[i] == P[k])
            next[i] = next[k];
        else next[i] = k;
    }
    return next;
}


int decode_xor() {

    int index = 0, demap_buf_index = 0;
    int file1index = 0, file2index;
    int prev_match_distance = 0;
    char *demapbuf = new char[frame_len];
    string buf3bits, out3bits, outbits;
    while (true) {
        sem_wait(&buf1_ready);
        sem_wait(&buf2_ready);
        sem_wait(&buf1_mutex);
        sem_wait(&buf2_mutex);
        int file2_offset = find_first_match(buf1 + index, buf2, 15, buf2_index);
        int match_distance = index - file2_offset;

        cout << "file1_offset=" << index << "; " << "match=" << file2_offset;
        if (file2_offset > 0) {
            if (match_distance != prev_match_distance) {
                //resync
                out3bits += buf3bits;
                out3bits += "\n";
                buf3bits = "";
                cout << endl;
            }
            cout << "  " << match_distance << endl;
            int file1_match_index = index;
            //begin to xor
            file1index = index;
            file2index = file2_offset;
            cout << "Decode from " << file1index << " to ";
            while (file1index < buf1_index && file2index < buf2_index && file1index < file1_match_index + RESYNC_LEN) {
                if (buf1[file1index] == buf2[file2index]) {
                    demapbuf[demap_buf_index] = 0;
                } else {
                    demapbuf[demap_buf_index] = 1;
                }
                file1index++;
                file2index++;
                demap_buf_index++;
                if (demap_buf_index == frame_len) {
                    int cnt_one = cntones(demapbuf, frame_len);
                    double one_proportion = (double) cnt_one / frame_len;
                    if (one_proportion > ones_threshold) {
                        buf3bits += "1";
                        cout << "1";
                    } else {
                        buf3bits += "0";
                        cout << "0";
                    }
                    demap_buf_index = 0;
                }
            }
            cout << file1index << endl;
            index = file1index;
            prev_match_distance = match_distance;
        } else {
            cout << endl;
            index += next_match_interval;
        }
        if(cur_decode_index>POPBUFLEN && prevmatch>POPBUFLEN){

        }
        sem_post(&buf1_mutex);
        sem_post(&buf2_mutex);
    }
    return out3bits.size();
}

void read_test(int fd){
    cout<<"Function read_test"<<endl;
    size_t nread;
    while(true){
//        sem_wait(&buf1_mutex);
        nread = read(fd, buf1 + buf1_index, BUFSIZE);
        if(nread<0){
            cout<<"error "<<nread<<endl;
            break;
        }
        buf1_index += nread;
        cout<<"Buf1 read bytes "<<buf1_index<<endl;
//        sem_post(&buf1_mutex);
    }
}

void* read_buf1(void* args){
    int fd = *((int*)args);
    int cnt = 0;
    pthread_detach(pthread_self());
    printf("Function read_buf1\n");
    FILE* fp = fopen("buf1.txt", "w");
    size_t nread = 0;
    int i=0;
    while(true){
//        sem_wait(&buf1_mutex);
        nread = read(fd, buf1 + buf1_index, BUFSIZE);
        if(nread<0){
            fprintf(fp, "error %d\n", (int)nread);
            break;
        }
        buf1_index += nread;
        cnt += nread;
        fprintf(fp, "Buf1 read bytes %d, nread = %d, i = %d\n", cnt, (int)nread, i);
        fflush(fp);
        i++;
//        sem_post(&buf1_mutex);
        // if(buf1_index - cur_decode_index > RESYNC_LEN){
        //     sem_post(&buf1_ready);
        // }
    }
}

void* read_buf2(void* args){
    int fd = *((int*)args);
    int cnt = 0;
    pthread_detach(pthread_self());
    printf("Function read_buf2\n");
    FILE* fp = fopen("buf2.txt", "w");
    ssize_t nread;
    int i=0;
    while(true){
//        sem_wait(&buf2_mutex);
        nread = read(fd, buf2 + buf2_index, BUFSIZE);
        if(nread<0){
            fprintf(fp, "error %d\n", (int)nread);
            break;
        }
        buf2_index += nread;
        cnt += nread;
        fprintf(fp, "Buf2 read bytes %d, nread = %d, i = %d\n", cnt, (int)nread, i);
        fflush(fp);
        i++;
//        sem_post(&buf2_mutex);
        // if(buf2_index - cur_decode_index > min_match_distance){
        //     sem_post(&buf2_ready);
        // }
    }
}

void decode_main(int fd1, int fd2){
    printf("Function decode_main\n");
    cout<<fd1<<' '<<fd2<<endl;
    void* status;
    // sem_init(&buf1_mutex, 0, 1);
    // sem_init(&buf2_mutex, 0, 1);
    // sem_init(&buf1_ready, 0, 0);
    // sem_init(&buf2_ready, 0, 0);
    pthread_t tid1, tid2;
    pthread_create(&tid1, NULL, read_buf1, &fd1);
    pthread_create(&tid2, NULL, read_buf2, &fd2);
    pthread_join(tid1, &status);
    pthread_join(tid2, &status);
}
}


