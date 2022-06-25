/*******************************************************************************
 *      MSM -- Mini Stack Machine
 *
 * Copyright (c) 2004-2018  LIMSI / CNRS
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 ******************************************************************************/
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define swap(a, b, t) do {         \
    t tmp = a; a = b; b = tmp; \
} while (0)

static
void error(int lno, const char *msg, ...) {
    va_list args;
    if (lno != -1) fprintf(stderr, "error[%d]: ", lno);
    else           fprintf(stderr, "error: ");
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);
    fprintf(stderr, "\n");
    exit(EXIT_FAILURE);
}

enum {
    op_drop,      op_dup,       op_push,      op_get,       op_set,
    op_read,      op_write,     op_add,       op_sub,       op_mul,
    op_div,       op_mod,       op_not,       op_and,       op_or,
    op_cmpeq,     op_cmpne,     op_cmplt,     op_cmple,     op_cmpgt,
    op_cmpge,     op_jump,      op_jumpt,     op_jumpf,     op_prep,
    op_call,      op_ret,       op_resn,      op_send,      op_recv,
    op_dbg,       op_halt
};
struct {
    char *name;
    int   type;
} static const opd[] = {
    {"drop",  0}, {"dup",   0}, {"push",  1}, {"get",   1}, {"set",   1},
    {"read",  0}, {"write", 0}, {"add",   0}, {"sub",   0}, {"mul",   0},
    {"div",   0}, {"mod",   0}, {"not",   0}, {"and",   0}, {"or",    0},
    {"cmpeq", 0}, {"cmpne", 0}, {"cmplt", 0}, {"cmple", 0}, {"cmpgt", 0},
    {"cmpge", 0}, {"jump",  2}, {"jumpt", 2}, {"jumpf", 2}, {"prep",  2},
    {"call",  1}, {"ret",   0}, {"resn",  1}, {"send",  0}, {"recv",  0},
    {"dbg",   0}, {"halt",  0}
};

typedef struct lbl_s lbl_t;
struct lbl_s {
    lbl_t *next;
    char  *name;
    int    addr;
    int    link;
} static *lbl = NULL;

static
lbl_t *label(const char *name) {
    lbl_t *res = lbl;
    while (res) {
        if (!strcmp(res->name, name))
            return res;
        res = res->next;
    }
    res = malloc(sizeof(lbl_t));
    res->name = malloc(strlen(name) + 1);
    strcpy(res->name, name);
    res->addr = res->link = -1;
    res->next = lbl;
    lbl = res;
    return res;
}

int main(int argc, char *argv[]) {
    int N = 1 << 16, dbg = 0, i;
    int *mem, pc, sp, bp;
    FILE *file = stdin;
    argc--, argv++;
    while (argc > 0) {
             if (!strcmp(argv[0], "-d")) dbg++;
        else if (!strcmp(argv[0], "-m")) N = 1 << 24;
        else if ((file = fopen(argv[0], "r")) == 0)
            error(-1, "cannot open input file");
        argc--, argv++;
    }
    if (!(mem = malloc(sizeof(int) * N)))
        error(-1, "not enough memory");
    pc = 1; memset(mem, 0, sizeof(int) * N);
    do {
        int lno = 0;
        while (!feof(file)) {
            int cnt = 0, opc = -1;
            char buf[4096], *tok[16], *ln = buf;
            if (!fgets(buf, sizeof(buf), file))
                break;
            lno++;
            while (*ln) {
                while (*ln &&  isspace(*ln)) ln++;
                if (*ln == '\0' || *ln == ';') break;
                tok[cnt++] = ln;
                while (*ln && !isspace(*ln)) ln++;
                if (*ln == '\0')               break;
                *ln++ = '\0';
            }
            if (cnt && tok[0][0] == '.') {
                lbl_t *l = label(tok[0] + 1);
                for (i = 1; i < cnt; i++) tok[i - 1] = tok[i];
                if (l->addr != -1) error(lno, "invalid label");
                l->addr = pc; cnt--;
            }
            if (cnt == 0) continue;
            for (i = 0; opc == -1 && i <= op_halt; i++)
                if (!strcmp(tok[0], opd[i].name))
                    opc = i;
            if (opc == -1)
                error(lno, "unknown opcode <%s>", tok[0]);
            mem[pc++] = opc;
            if (opd[opc].type == 0) {
                if (cnt != 1)
                    error(lno, "too much arguments");
            } else {
                if (cnt != 2) error(lno, "invalid arg count");
                if (opd[opc].type == 1) {
                    mem[pc++] = atoi(tok[1]);
                } else if (opd[opc].type == 2) {
                    lbl_t *l = label(tok[1]);
                    mem[pc++] = l->link;
                    l->link = pc - 1;
                }
            }
        }
    } while (0);
    mem[0] = pc;
    do {
        lbl_t *l;
        for (l = lbl; l != 0; l = l->next) {
            if (l->addr == -1)
                error(-1, "undefined label <%s>", l->name);
            for (i = l->link; i != -1; ) {
                const int tmp = mem[i];
                mem[i] = l->addr;
                i = tmp;
            }
        }
    } while (0);
    if ((pc = label("start")->addr) < 0)
        error(-1, "start not defined");
    sp = N; bp = N;
    while (1) {
        const int tp = sp, nx = sp + 1;
        const int opc = mem[pc++];
        if (dbg) {
            if (dbg > 1) {
                printf("\nBP=%d\n", bp);
                for (i = N - 1; i >= sp; i--)
                    printf("  STK[%d] = %d\n", i, mem[i]);
            }
            printf("  MEM[%d] %s", pc - 1, opd[opc].name);
            if (opd[opc].type) printf(" %d", mem[pc]);
            printf("\n");
        }
        switch (opc) {
        case op_drop:   sp++;                                break;
        case op_dup:    mem[--sp] = mem[tp];                 break;
        case op_push:   mem[--sp] = mem[pc++];               break;
        case op_get:    mem[--sp] = mem[bp - mem[pc++] - 1]; break;
        case op_set:    mem[bp - mem[pc++] - 1] = mem[sp++]; break;
        case op_read:   mem[tp] = mem[mem[tp]];              break;
        case op_write:  mem[mem[tp]] = mem[nx]; sp += 2;     break;
        case op_add:    mem[nx] = mem[nx] +  mem[tp]; sp++;  break;
        case op_sub:    mem[nx] = mem[nx] -  mem[tp]; sp++;  break;
        case op_mul:    mem[nx] = mem[nx] *  mem[tp]; sp++;  break;
        case op_div:    mem[nx] = mem[nx] /  mem[tp]; sp++;  break;
        case op_mod:    mem[nx] = mem[nx] %  mem[tp]; sp++;  break;
        case op_not:    mem[tp] =!mem[tp];                   break;
        case op_and:    mem[nx] = mem[nx] && mem[tp]; sp++;  break;
        case op_or:     mem[nx] = mem[nx] || mem[tp]; sp++;  break;
        case op_cmpeq:  mem[nx] = mem[nx] == mem[tp]; sp++;  break;
        case op_cmpne:  mem[nx] = mem[nx] != mem[tp]; sp++;  break;
        case op_cmplt:  mem[nx] = mem[nx] <  mem[tp]; sp++;  break;
        case op_cmple:  mem[nx] = mem[nx] <= mem[tp]; sp++;  break;
        case op_cmpgt:  mem[nx] = mem[nx] >  mem[tp]; sp++;  break;
        case op_cmpge:  mem[nx] = mem[nx] >= mem[tp]; sp++;  break;
        case op_jump:   pc =                 mem[pc];        break;
        case op_jumpt:  pc = ( mem[sp++] ? mem[pc] : pc+1);  break;
        case op_jumpf:  pc = (!mem[sp++] ? mem[pc] : pc+1);  break;
        case op_prep:   mem[--sp] = mem[pc++];
                        mem[--sp] = bp;                      break;
        case op_call:   bp = sp + mem[pc++];
                        swap(mem[bp+1], pc, int);            break;
        case op_ret:    pc = mem[bp+1]; mem[bp+1] = mem[sp];
                        sp = bp; bp = mem[sp++];             break;
        case op_resn:   sp -= mem[pc++];                     break;
        case op_send:   printf("%c", mem[sp++]);             break;
        case op_recv:   mem[--sp] = getchar();               break;
        case op_dbg:    printf("%d\n", mem[sp++]);           break;
        case op_halt:   goto halt;
        }
    }
    halt:
    return EXIT_SUCCESS;
}

/*******************************************************************************
 * This is the end...
 ******************************************************************************/
