.data
    .global _counterPeriodicTask
_counterPeriodicTask:
.long 0x6f4
    .global status
status:
.long 0x0
    .global batV
batV:
.long 0x0
    .global entry
.text
.bss
entry.9:
    .space 4
.text
entry:
	move r2,_counterPeriodicTask
	ld r1,r2,0
	add r1,r1,1
	st r1,r2,0
	ld r2,r2,0
	move r1,1800
	sub r2,r2,r1 #{ if r2 < r1 goto L.2
	add r2,r2,r1
	jump L.2, ov #}
	move r2,_counterPeriodicTask
	move r1,0
	st r1,r2,0
	move r2,status
	ld r1,r2,0
	or r1,r1,1
	st r1,r2,0
	wake 
	jump L.1
L.2:
	move r2,batV
	ld r2,r2,0
	move r2,r2 #{ if r2 goto L.4 
	jump 1f, eq
	jump L.4
1:           #}
	adc r0,0,7
	move r1,batV
	st r0,r1,0
L.4:
	adc r0,0,7
	move r2,r0
	move r1,batV
	move r0,batV
	move r3,entry.9
	st r0,r3,0
	rsh r2,r2,1
	ld r0,r1,0
	rsh r0,r0,1
	add r2,r2,r0
	move r0,entry.9
	ld r0,r0,0
	st r2,r0,0
	ld r2,r1,0
	move r1,2158
	sub r2,r2,r1 #{ if r2 < r1 goto L.6
	add r2,r2,r1
	jump L.6, ov #}
	move r2,status
	ld r1,r2,0
	or r1,r1,4
	st r1,r2,0
	wake 
	jump L.1
L.6:
	move r2,batV
	ld r2,r2,0
	move r1,2019
	sub r1,r1,r2 #{ if r2 > r1 goto L.8
	add r1,r1,r2
	jump L.8, ov #}
	move r2,status
	ld r2,r2,0
	and r2,r2,2
	move r2,r2 #{ if r2 goto L.8 
	jump 1f, eq
	jump L.8
1:           #}
	move r2,status
	ld r1,r2,0
	or r1,r1,2
	st r1,r2,0
	wake 
L.8:
L.1:

halt
