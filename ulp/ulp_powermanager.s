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
    .global fileInd
fileInd:
.long 0x0
    .global frameIndH
frameIndH:
.long 0x0
    .global frameIndL
frameIndL:
.long 0x0
    .global entry
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
	adc r0,0,7
	move r1,batV
	st r0,r1,0
	move r2,batV
	ld r2,r2,0
	move r1,2251
	sub r2,r2,r1 #{ if r2 < r1 goto L.4
	add r2,r2,r1
	jump L.4, ov #}
	move r2,status
	ld r1,r2,0
	or r1,r1,4
	st r1,r2,0
	wake 
	jump L.1
L.4:
	move r2,batV
	ld r2,r2,0
	move r1,2020
	sub r1,r1,r2 #{ if r2 > r1 goto L.6
	add r1,r1,r2
	jump L.6, ov #}
	move r2,status
	ld r2,r2,0
	and r2,r2,2
	move r2,r2 #{ if r2 goto L.6 
	jump 1f, eq
	jump L.6
1:           #}
	move r2,status
	ld r1,r2,0
	or r1,r1,2
	st r1,r2,0
	wake 
L.6:
L.1:

halt
