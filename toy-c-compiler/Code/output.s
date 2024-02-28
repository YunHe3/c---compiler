.data
_ret: .asciiz "\n"
.text
_squaRever:
   addi $sp, $sp, -8
   sw $ra, 4($sp)
   sw $fp, 0($sp)
   addi $fp, $sp, 8
   addi $sp, $sp, -4
   lw $t0, -12($fp)
   sw $a0, -12($fp)
   addi $sp, $sp, -4
   lw $t0, -16($fp)
   li $t0, 0
   sw $t0, -16($fp)
   addi $sp, $sp, -4
   lw $t0, -20($fp)
   lw $t1, -16($fp)
   move $t0, $t1
   sw $t0, -20($fp)
   addi $sp, $sp, -4
   lw $t0, -24($fp)
   li $t0, 0
   sw $t0, -24($fp)
   addi $sp, $sp, -4
   lw $t0, -28($fp)
   lw $t1, -24($fp)
   move $t0, $t1
   sw $t0, -28($fp)
   addi $sp, $sp, -4
   lw $t0, -32($fp)
   addi $sp, $sp, -12
   lw $t1, -44($fp)
   addi $t0, $fp, -44
   sw $t0, -32($fp)
   addi $sp, $sp, -4
   lw $t0, -48($fp)
   li $t0, 0
   sw $t0, -48($fp)
   addi $sp, $sp, -4
   lw $t0, -52($fp)
   lw $t1, -48($fp)
   li $t3, 4
   mul $t0, $t1, $t3
   sw $t0, -52($fp)
   addi $sp, $sp, -4
   lw $t0, -56($fp)
   lw $t1, -32($fp)
   lw $t2, -52($fp)
   add $t0, $t1, $t2
   sw $t0, -56($fp)
   addi $sp, $sp, -4
   lw $t0, -60($fp)
   lw $t1, -56($fp)
   move $t0, $t1
   sw $t0, -60($fp)
   addi $sp, $sp, -4
   lw $t0, -64($fp)
   li $t0, 100
   sw $t0, -64($fp)
   addi $sp, $sp, -4
   lw $t0, -68($fp)
   lw $t1, -12($fp)
   lw $t2, -64($fp)
   div $t1, $t2
   mflo $t0
   sw $t0, -68($fp)
   lw $t0, -60($fp)
   lw $t1, -68($fp)
   sw $t1, 0($t0)
   addi $sp, $sp, -4
   lw $t0, -72($fp)
   lw $t1, -60($fp)
   lw $t0, 0($t1)
   sw $t0, -72($fp)
   addi $sp, $sp, -4
   lw $t0, -76($fp)
   lw $t1, -44($fp)
   addi $t0, $fp, -44
   sw $t0, -76($fp)
   addi $sp, $sp, -4
   lw $t0, -80($fp)
   li $t0, 1
   sw $t0, -80($fp)
   addi $sp, $sp, -4
   lw $t0, -84($fp)
   lw $t1, -80($fp)
   li $t3, 4
   mul $t0, $t1, $t3
   sw $t0, -84($fp)
   addi $sp, $sp, -4
   lw $t0, -88($fp)
   lw $t1, -76($fp)
   lw $t2, -84($fp)
   add $t0, $t1, $t2
   sw $t0, -88($fp)
   addi $sp, $sp, -4
   lw $t0, -92($fp)
   lw $t1, -88($fp)
   move $t0, $t1
   sw $t0, -92($fp)
   addi $sp, $sp, -4
   lw $t0, -96($fp)
   li $t0, 10
   sw $t0, -96($fp)
   addi $sp, $sp, -4
   lw $t0, -100($fp)
   lw $t1, -12($fp)
   lw $t2, -96($fp)
   div $t1, $t2
   mflo $t0
   sw $t0, -100($fp)
   addi $sp, $sp, -4
   lw $t0, -104($fp)
   li $t0, 10
   sw $t0, -104($fp)
   addi $sp, $sp, -4
   lw $t0, -108($fp)
   lw $t1, -44($fp)
   addi $t0, $fp, -44
   sw $t0, -108($fp)
   addi $sp, $sp, -4
   lw $t0, -112($fp)
   li $t0, 0
   sw $t0, -112($fp)
   addi $sp, $sp, -4
   lw $t0, -116($fp)
   lw $t1, -112($fp)
   li $t3, 4
   mul $t0, $t1, $t3
   sw $t0, -116($fp)
   addi $sp, $sp, -4
   lw $t0, -120($fp)
   lw $t1, -108($fp)
   lw $t2, -116($fp)
   add $t0, $t1, $t2
   sw $t0, -120($fp)
   addi $sp, $sp, -4
   lw $t0, -124($fp)
   lw $t1, -120($fp)
   lw $t0, 0($t1)
   sw $t0, -124($fp)
   addi $sp, $sp, -4
   lw $t0, -128($fp)
   lw $t1, -104($fp)
   lw $t2, -124($fp)
   mul $t0, $t1, $t2
   sw $t0, -128($fp)
   addi $sp, $sp, -4
   lw $t0, -132($fp)
   lw $t1, -100($fp)
   lw $t2, -128($fp)
   sub $t0, $t1, $t2
   sw $t0, -132($fp)
   lw $t0, -92($fp)
   lw $t1, -132($fp)
   sw $t1, 0($t0)
   addi $sp, $sp, -4
   lw $t0, -136($fp)
   lw $t1, -92($fp)
   lw $t0, 0($t1)
   sw $t0, -136($fp)
   addi $sp, $sp, -4
   lw $t0, -140($fp)
   lw $t1, -44($fp)
   addi $t0, $fp, -44
   sw $t0, -140($fp)
   addi $sp, $sp, -4
   lw $t0, -144($fp)
   li $t0, 2
   sw $t0, -144($fp)
   addi $sp, $sp, -4
   lw $t0, -148($fp)
   lw $t1, -144($fp)
   li $t3, 4
   mul $t0, $t1, $t3
   sw $t0, -148($fp)
   addi $sp, $sp, -4
   lw $t0, -152($fp)
   lw $t1, -140($fp)
   lw $t2, -148($fp)
   add $t0, $t1, $t2
   sw $t0, -152($fp)
   addi $sp, $sp, -4
   lw $t0, -156($fp)
   lw $t1, -152($fp)
   move $t0, $t1
   sw $t0, -156($fp)
   addi $sp, $sp, -4
   lw $t0, -160($fp)
   li $t0, 100
   sw $t0, -160($fp)
   addi $sp, $sp, -4
   lw $t0, -164($fp)
   lw $t1, -44($fp)
   addi $t0, $fp, -44
   sw $t0, -164($fp)
   addi $sp, $sp, -4
   lw $t0, -168($fp)
   li $t0, 0
   sw $t0, -168($fp)
   addi $sp, $sp, -4
   lw $t0, -172($fp)
   lw $t1, -168($fp)
   li $t3, 4
   mul $t0, $t1, $t3
   sw $t0, -172($fp)
   addi $sp, $sp, -4
   lw $t0, -176($fp)
   lw $t1, -164($fp)
   lw $t2, -172($fp)
   add $t0, $t1, $t2
   sw $t0, -176($fp)
   addi $sp, $sp, -4
   lw $t0, -180($fp)
   lw $t1, -176($fp)
   lw $t0, 0($t1)
   sw $t0, -180($fp)
   addi $sp, $sp, -4
   lw $t0, -184($fp)
   lw $t1, -160($fp)
   lw $t2, -180($fp)
   mul $t0, $t1, $t2
   sw $t0, -184($fp)
   addi $sp, $sp, -4
   lw $t0, -188($fp)
   lw $t1, -12($fp)
   lw $t2, -184($fp)
   sub $t0, $t1, $t2
   sw $t0, -188($fp)
   addi $sp, $sp, -4
   lw $t0, -192($fp)
   li $t0, 10
   sw $t0, -192($fp)
   addi $sp, $sp, -4
   lw $t0, -196($fp)
   lw $t1, -44($fp)
   addi $t0, $fp, -44
   sw $t0, -196($fp)
   addi $sp, $sp, -4
   lw $t0, -200($fp)
   li $t0, 1
   sw $t0, -200($fp)
   addi $sp, $sp, -4
   lw $t0, -204($fp)
   lw $t1, -200($fp)
   li $t3, 4
   mul $t0, $t1, $t3
   sw $t0, -204($fp)
   addi $sp, $sp, -4
   lw $t0, -208($fp)
   lw $t1, -196($fp)
   lw $t2, -204($fp)
   add $t0, $t1, $t2
   sw $t0, -208($fp)
   addi $sp, $sp, -4
   lw $t0, -212($fp)
   lw $t1, -208($fp)
   lw $t0, 0($t1)
   sw $t0, -212($fp)
   addi $sp, $sp, -4
   lw $t0, -216($fp)
   lw $t1, -192($fp)
   lw $t2, -212($fp)
   mul $t0, $t1, $t2
   sw $t0, -216($fp)
   addi $sp, $sp, -4
   lw $t0, -220($fp)
   lw $t1, -188($fp)
   lw $t2, -216($fp)
   sub $t0, $t1, $t2
   sw $t0, -220($fp)
   lw $t0, -156($fp)
   lw $t1, -220($fp)
   sw $t1, 0($t0)
   addi $sp, $sp, -4
   lw $t0, -224($fp)
   lw $t1, -156($fp)
   lw $t0, 0($t1)
   sw $t0, -224($fp)
   addi $sp, $sp, -4
   lw $t0, -228($fp)
   lw $t1, -44($fp)
   addi $t0, $fp, -44
   sw $t0, -228($fp)
   addi $sp, $sp, -4
   lw $t0, -232($fp)
   li $t0, 0
   sw $t0, -232($fp)
   addi $sp, $sp, -4
   lw $t0, -236($fp)
   lw $t1, -232($fp)
   li $t3, 4
   mul $t0, $t1, $t3
   sw $t0, -236($fp)
   addi $sp, $sp, -4
   lw $t0, -240($fp)
   lw $t1, -228($fp)
   lw $t2, -236($fp)
   add $t0, $t1, $t2
   sw $t0, -240($fp)
   addi $sp, $sp, -4
   lw $t0, -244($fp)
   lw $t1, -240($fp)
   lw $t0, 0($t1)
   sw $t0, -244($fp)
   addi $sp, $sp, -4
   lw $t0, -248($fp)
   lw $t1, -44($fp)
   addi $t0, $fp, -44
   sw $t0, -248($fp)
   addi $sp, $sp, -4
   lw $t0, -252($fp)
   li $t0, 2
   sw $t0, -252($fp)
   addi $sp, $sp, -4
   lw $t0, -256($fp)
   lw $t1, -252($fp)
   li $t3, 4
   mul $t0, $t1, $t3
   sw $t0, -256($fp)
   addi $sp, $sp, -4
   lw $t0, -260($fp)
   lw $t1, -248($fp)
   lw $t2, -256($fp)
   add $t0, $t1, $t2
   sw $t0, -260($fp)
   addi $sp, $sp, -4
   lw $t0, -264($fp)
   lw $t1, -260($fp)
   lw $t0, 0($t1)
   sw $t0, -264($fp)
   lw $t0, -244($fp)
   lw $t1, -264($fp)
   bne $t0, $t1, label1
   j label2
label1:
   addi $sp, $sp, -4
   lw $t0, -268($fp)
   li $t0, 0
   sw $t0, -268($fp)
   lw $t0, -20($fp)
   lw $t1, -268($fp)
   move $t0, $t1
   sw $t0, -20($fp)
   addi $sp, $sp, -4
   lw $t0, -272($fp)
   lw $t1, -20($fp)
   move $t0, $t1
   sw $t0, -272($fp)
   j label3
label2:
label6:
   addi $sp, $sp, -4
   lw $t0, -276($fp)
   li $t0, 12
   sw $t0, -276($fp)
   lw $t0, -28($fp)
   lw $t1, -276($fp)
   blt $t0, $t1, label4
   j label5
label4:
   addi $sp, $sp, -4
   lw $t0, -280($fp)
   lw $t1, -28($fp)
   lw $t2, -28($fp)
   mul $t0, $t2, $t2
   sw $t0, -280($fp)
   addi $sp, $sp, -4
   lw $t0, -284($fp)
   lw $t1, -280($fp)
   lw $t2, -12($fp)
   sub $t0, $t1, $t2
   sw $t0, -284($fp)
   addi $sp, $sp, -4
   lw $t0, -288($fp)
   li $t0, 0
   sw $t0, -288($fp)
   lw $t0, -284($fp)
   lw $t1, -288($fp)
   beq $t0, $t1, label7
   j label8
label7:
   addi $sp, $sp, -4
   lw $t0, -292($fp)
   li $t0, 1
   sw $t0, -292($fp)
   lw $t0, -20($fp)
   lw $t1, -292($fp)
   move $t0, $t1
   sw $t0, -20($fp)
   addi $sp, $sp, -4
   lw $t0, -296($fp)
   lw $t1, -20($fp)
   move $t0, $t1
   sw $t0, -296($fp)
label8:
   addi $sp, $sp, -4
   lw $t0, -300($fp)
   li $t0, 1
   sw $t0, -300($fp)
   addi $sp, $sp, -4
   lw $t0, -304($fp)
   lw $t1, -28($fp)
   lw $t2, -300($fp)
   add $t0, $t1, $t2
   sw $t0, -304($fp)
   lw $t0, -28($fp)
   lw $t1, -304($fp)
   move $t0, $t1
   sw $t0, -28($fp)
   addi $sp, $sp, -4
   lw $t0, -308($fp)
   lw $t1, -28($fp)
   move $t0, $t1
   sw $t0, -308($fp)
   j label6
label5:
label3:
   addi $sp, $sp, -4
   lw $t0, -312($fp)
   li $t0, 1
   sw $t0, -312($fp)
   lw $t0, -20($fp)
   lw $t1, -312($fp)
   beq $t0, $t1, label9
   j label10
label9:
   addi $sp, $sp, -4
   lw $t0, -316($fp)
   li $t0, 1
   sw $t0, -316($fp)
   lw $t0, -316($fp)
   move $v0, $t0
   move $sp, $fp
   lw $ra, -4($fp)
   lw $fp, -8($fp)
   jr $ra
   j label11
label10:
   addi $sp, $sp, -4
   lw $t0, -320($fp)
   li $t0, 0
   sw $t0, -320($fp)
   lw $t0, -320($fp)
   move $v0, $t0
   move $sp, $fp
   lw $ra, -4($fp)
   lw $fp, -8($fp)
   jr $ra
label11:
main:
   addi $sp, $sp, -8
   sw $ra, 4($sp)
   sw $fp, 0($sp)
   addi $fp, $sp, 8
   addi $sp, $sp, -4
   lw $t0, -12($fp)
   li $t0, 100
   sw $t0, -12($fp)
   addi $sp, $sp, -4
   lw $t0, -16($fp)
   lw $t1, -12($fp)
   move $t0, $t1
   sw $t0, -16($fp)
label14:
   addi $sp, $sp, -4
   lw $t0, -20($fp)
   li $t0, 150
   sw $t0, -20($fp)
   lw $t0, -16($fp)
   lw $t1, -20($fp)
   blt $t0, $t1, label12
   j label13
label12:
   lw $t0, -16($fp)
   lw $a0, -16($fp)
   addi $sp, $sp, -4
   lw $t0, -24($fp)
   jal _squaRever
   move $t0, $v0
   sw $t0, -24($fp)
   addi $sp, $sp, 0
   addi $sp, $sp, -4
   lw $t0, -28($fp)
   li $t0, 1
   sw $t0, -28($fp)
   lw $t0, -24($fp)
   lw $t1, -28($fp)
   beq $t0, $t1, label15
   j label16
label15:
   lw $t0, -16($fp)
   li $v0, 1
   move $a0, $t0
   syscall
   li $v0, 4
   la $a0, _ret
   syscall
   addi $sp, $sp, -4
   lw $t0, -32($fp)
   li $t0, 0
   sw $t0, -32($fp)
label16:
   addi $sp, $sp, -4
   lw $t0, -36($fp)
   li $t0, 1
   sw $t0, -36($fp)
   addi $sp, $sp, -4
   lw $t0, -40($fp)
   lw $t1, -16($fp)
   lw $t2, -36($fp)
   add $t0, $t1, $t2
   sw $t0, -40($fp)
   lw $t0, -16($fp)
   lw $t1, -40($fp)
   move $t0, $t1
   sw $t0, -16($fp)
   addi $sp, $sp, -4
   lw $t0, -44($fp)
   lw $t1, -16($fp)
   move $t0, $t1
   sw $t0, -44($fp)
   j label14
label13:
   addi $sp, $sp, -4
   lw $t0, -48($fp)
   li $t0, 0
   sw $t0, -48($fp)
   lw $t0, -48($fp)
   move $v0, $t0
   move $sp, $fp
   lw $ra, -4($fp)
   lw $fp, -8($fp)
   jr $ra
   li $v0, 10
   syscall
