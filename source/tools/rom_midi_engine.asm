    86ee:	4e56 fffc      	linkw %fp,#-4
    86f2:	206e 0008      	moveal %fp@(8),%a0
    86f6:	1028 0002      	moveb %a0@(2),%d0
    86fa:	4880           	extw %d0
    86fc:	33c0 0001 20fe 	movew %d0,0x120fe
    8702:	3eb9 0001 20fe 	movew 0x120fe,%sp@
    8708:	6100 036e      	bsrw 0x8a78
    870c:	202e 0008      	movel %fp@(8),%d0
    8710:	5680           	addql #3,%d0
    8712:	4e5e           	unlk %fp
    8714:	4e75           	rts
    8716:	4e56 fffc      	linkw %fp,#-4
    871a:	206e 0008      	moveal %fp@(8),%a0
    871e:	1028 0001      	moveb %a0@(1),%d0
    8722:	4880           	extw %d0
    8724:	33c0 0001 2102 	movew %d0,0x12102
    872a:	303c 0960      	movew #2400,%d0
    872e:	48c0           	extl %d0
    8730:	81f9 0001 2102 	divsw 0x12102,%d0
    8736:	33c0 0001 2100 	movew %d0,0x12100
    873c:	202e 0008      	movel %fp@(8),%d0
    8740:	5480           	addql #2,%d0
    8742:	4e5e           	unlk %fp
    8744:	4e75           	rts
    8746:	4e56 fffc      	linkw %fp,#-4
    874a:	202e 0008      	movel %fp@(8),%d0
    874e:	202e 0008      	movel %fp@(8),%d0
    8752:	5480           	addql #2,%d0
    8754:	4e5e           	unlk %fp
    8756:	4e75           	rts
    8758:	4e56 fffc      	linkw %fp,#-4
    875c:	202e 0008      	movel %fp@(8),%d0
    8760:	202e 0008      	movel %fp@(8),%d0
    8764:	5680           	addql #3,%d0
    8766:	4e5e           	unlk %fp
    8768:	4e75           	rts
    876a:	4e56 fffc      	linkw %fp,#-4
    876e:	202e 0008      	movel %fp@(8),%d0
    8772:	5680           	addql #3,%d0
    8774:	4e5e           	unlk %fp
    8776:	4e75           	rts
    8778:	4e56 fff8      	linkw %fp,#-8
    877c:	4a79 0001 1780 	tstw 0x11780
    8782:	670a           	beqs 0x878e
    8784:	33fc 0002 0001 	movew #2,0x120e0
    878a:	20e0 
    878c:	604a           	bras 0x87d8
    878e:	202e 0008      	movel %fp@(8),%d0
    8792:	d0bc 0000 01fe 	addl #510,%d0
    8798:	23c0 0001 20e2 	movel %d0,0x120e2
    879e:	2eb9 0001 20e2 	movel 0x120e2,%sp@
    87a4:	6136           	bsrs 0x87dc
    87a6:	6100 010e      	bsrw 0x88b6
    87aa:	2eae 000c      	movel %fp@(12),%sp@
    87ae:	2f39 0001 20e2 	movel 0x120e2,%sp@-
    87b4:	6100 016e      	bsrw 0x8924
    87b8:	588f           	addql #4,%sp
    87ba:	2d40 fffc      	movel %d0,%fp@(-4)
    87be:	2eae 000c      	movel %fp@(12),%sp@
    87c2:	2f2e fffc      	movel %fp@(-4),%sp@-
    87c6:	6100 01a4      	bsrw 0x896c
    87ca:	588f           	addql #4,%sp
    87cc:	6100 0200      	bsrw 0x89ce
    87d0:	33fc 0001 0001 	movew #1,0x11780
    87d6:	1780 
    87d8:	4e5e           	unlk %fp
    87da:	4e75           	rts
    87dc:	4e56 fffc      	linkw %fp,#-4
    87e0:	2eae 0008      	movel %fp@(8),%sp@
    87e4:	0697 ffff ffa6 	addil #-90,%sp@
    87ea:	6100 022e      	bsrw 0x8a1a
    87ee:	206e 0008      	moveal %fp@(8),%a0
    87f2:	4a10           	tstb %a0@
    87f4:	660a           	bnes 0x8800
    87f6:	202e 0008      	movel %fp@(8),%d0
    87fa:	5280           	addql #1,%d0
    87fc:	2d40 0008      	movel %d0,%fp@(8)
    8800:	206e 0008      	moveal %fp@(8),%a0
    8804:	4a10           	tstb %a0@
    8806:	6700 00aa      	beqw 0x88b2
    880a:	206e 0008      	moveal %fp@(8),%a0
    880e:	1010           	moveb %a0@,%d0
    8810:	4880           	extw %d0
    8812:	c07c 009f      	andw #159,%d0
    8816:	b07c 0020      	cmpw #32,%d0
    881a:	6c1c           	bges 0x8838
    881c:	206e 0008      	moveal %fp@(8),%a0
    8820:	1010           	moveb %a0@,%d0
    8822:	4880           	extw %d0
    8824:	c07c 009f      	andw #159,%d0
    8828:	670e           	beqs 0x8838
    882a:	202e 0008      	movel %fp@(8),%d0
    882e:	5680           	addql #3,%d0
    8830:	2d40 0008      	movel %d0,%fp@(8)
    8834:	6000 0078      	braw 0x88ae
    8838:	206e 0008      	moveal %fp@(8),%a0
    883c:	1010           	moveb %a0@,%d0
    883e:	4880           	extw %d0
    8840:	6056           	bras 0x8898
    8842:	2eae 0008      	movel %fp@(8),%sp@
    8846:	6100 fea6      	bsrw 0x86ee
    884a:	2d40 0008      	movel %d0,%fp@(8)
    884e:	605e           	bras 0x88ae
    8850:	2eae 0008      	movel %fp@(8),%sp@
    8854:	6100 fec0      	bsrw 0x8716
    8858:	2d40 0008      	movel %d0,%fp@(8)
    885c:	6050           	bras 0x88ae
    885e:	2eae 0008      	movel %fp@(8),%sp@
    8862:	6100 fee2      	bsrw 0x8746
    8866:	2d40 0008      	movel %d0,%fp@(8)
    886a:	6042           	bras 0x88ae
    886c:	2eae 0008      	movel %fp@(8),%sp@
    8870:	6100 fee6      	bsrw 0x8758
    8874:	2d40 0008      	movel %d0,%fp@(8)
    8878:	6034           	bras 0x88ae
    887a:	2eae 0008      	movel %fp@(8),%sp@
    887e:	6100 feea      	bsrw 0x876a
    8882:	2d40 0008      	movel %d0,%fp@(8)
    8886:	6026           	bras 0x88ae
    8888:	6028           	bras 0x88b2
    888a:	202e 0008      	movel %fp@(8),%d0
    888e:	5280           	addql #1,%d0
    8890:	2d40 0008      	movel %d0,%fp@(8)
    8894:	6018           	bras 0x88ae
    8896:	6016           	bras 0x88ae
    8898:	48c0           	extl %d0
    889a:	207c 0001 2a3e 	moveal #76350,%a0
    88a0:	7206           	moveq #6,%d1
    88a2:	b098           	cmpl %a0@+,%d0
    88a4:	57c9 fffc      	dbeq %d1,0x88a2
    88a8:	2068 0018      	moveal %a0@(24),%a0
    88ac:	4ed0           	jmp %a0@
    88ae:	6000 ff50      	braw 0x8800
    88b2:	4e5e           	unlk %fp
    88b4:	4e75           	rts
    88b6:	4e56 fff8      	linkw %fp,#-8
    88ba:	426e fffe      	clrw %fp@(-2)
    88be:	6058           	bras 0x8918
    88c0:	3d7c 0001 fffc 	movew #1,%fp@(-4)
    88c6:	603e           	bras 0x8906
    88c8:	306e fffc      	moveaw %fp@(-4),%a0
    88cc:	227c 0004 8bce 	moveal #297934,%a1
    88d2:	1030 9800      	moveb %a0@(0,%a1:l),%d0
    88d6:	4880           	extw %d0
    88d8:	c07c 000f      	andw #15,%d0
    88dc:	b06e fffe      	cmpw %fp@(-2),%d0
    88e0:	661a           	bnes 0x88fc
    88e2:	306e fffc      	moveaw %fp@(-4),%a0
    88e6:	d1c8           	addal %a0,%a0
    88e8:	d1fc 0003 df96 	addal #253846,%a0
    88ee:	30bc ffff      	movew #-1,%a0@
    88f2:	3eae fffc      	movew %fp@(-4),%sp@
    88f6:	6100 032c      	bsrw 0x8c24
    88fa:	6012           	bras 0x890e
    88fc:	302e fffc      	movew %fp@(-4),%d0
    8900:	5240           	addqw #1,%d0
    8902:	3d40 fffc      	movew %d0,%fp@(-4)
    8906:	0c6e 0010 fffc 	cmpiw #16,%fp@(-4)
    890c:	6dba           	blts 0x88c8
    890e:	302e fffe      	movew %fp@(-2),%d0
    8912:	5240           	addqw #1,%d0
    8914:	3d40 fffe      	movew %d0,%fp@(-2)
    8918:	0c6e 0010 fffe 	cmpiw #16,%fp@(-2)
    891e:	6da0           	blts 0x88c0
    8920:	4e5e           	unlk %fp
    8922:	4e75           	rts
    8924:	4e56 fffc      	linkw %fp,#-4
    8928:	202e 000c      	movel %fp@(12),%d0
    892c:	4aae 0008      	tstl %fp@(8)
    8930:	6604           	bnes 0x8936
    8932:	4280           	clrl %d0
    8934:	6032           	bras 0x8968
    8936:	206e 0008      	moveal %fp@(8),%a0
    893a:	4a10           	tstb %a0@
    893c:	6612           	bnes 0x8950
    893e:	206e 0008      	moveal %fp@(8),%a0
    8942:	0c28 00ff 0001 	cmpib #-1,%a0@(1)
    8948:	6606           	bnes 0x8950
    894a:	202e 0008      	movel %fp@(8),%d0
    894e:	6018           	bras 0x8968
    8950:	600a           	bras 0x895c
    8952:	202e 0008      	movel %fp@(8),%d0
    8956:	5280           	addql #1,%d0
    8958:	2d40 0008      	movel %d0,%fp@(8)
    895c:	206e 0008      	moveal %fp@(8),%a0
    8960:	4a10           	tstb %a0@
    8962:	66ee           	bnes 0x8952
    8964:	202e 0008      	movel %fp@(8),%d0
    8968:	4e5e           	unlk %fp
    896a:	4e75           	rts
    896c:	4e56 fffc      	linkw %fp,#-4
    8970:	23ee 0008 0001 	movel %fp@(8),0x120e6
    8976:	20e6 
    8978:	4aae 000c      	tstl %fp@(12)
    897c:	6604           	bnes 0x8982
    897e:	70ff           	moveq #-1,%d0
    8980:	6004           	bras 0x8986
    8982:	202e 000c      	movel %fp@(12),%d0
    8986:	23c0 0001 20ea 	movel %d0,0x120ea
    898c:	2039 0001 20e2 	movel 0x120e2,%d0
    8992:	d0bc ffff fe98 	addl #-360,%d0
    8998:	23c0 0001 20ee 	movel %d0,0x120ee
    899e:	33f9 0001 20f4 	movew 0x120f4,0x120f2
    89a4:	0001 20f2 
    89a8:	33f9 0001 20f8 	movew 0x120f8,0x120f6
    89ae:	0001 20f6 
    89b2:	4279 0001 20fa 	clrw 0x120fa
    89b8:	33fc 0009 0001 	movew #9,0x120fc
    89be:	20fc 
    89c0:	33f9 0001 2100 	movew 0x12100,0x1c642
    89c6:	0001 c642 
    89ca:	4e5e           	unlk %fp
    89cc:	4e75           	rts
    89ce:	4e56 fffc      	linkw %fp,#-4
    89d2:	42b9 0001 2104 	clrl 0x12104
    89d8:	4279 0001 2108 	clrw 0x12108
    89de:	33fc 0064 0001 	movew #100,0x1210a
    89e4:	210a 
    89e6:	33fc 0064 0001 	movew #100,0x1210c
    89ec:	210c 
    89ee:	33fc 0064 0001 	movew #100,0x1210e
    89f4:	210e 
    89f6:	33fc 0064 0001 	movew #100,0x12110
    89fc:	2110 
    89fe:	33fc 0064 0001 	movew #100,0x12112
    8a04:	2112 
    8a06:	33fc 0001 0001 	movew #1,0x12114
    8a0c:	2114 
    8a0e:	33fc 0001 0001 	movew #1,0x120e0
    8a14:	20e0 
    8a16:	4e5e           	unlk %fp
    8a18:	4e75           	rts
    8a1a:	4e56 fffa      	linkw %fp,#-6
    8a1e:	3d7c 0001 fffe 	movew #1,%fp@(-2)
    8a24:	6046           	bras 0x8a6c
    8a26:	206e 0008      	moveal %fp@(8),%a0
    8a2a:	326e fffe      	moveaw %fp@(-2),%a1
    8a2e:	1030 90ff      	moveb %a0@(ffffffffffffffff,%a1:w),%d0
    8a32:	4880           	extw %d0
    8a34:	5340           	subqw #1,%d0
    8a36:	227c 0004 8bce 	moveal #297934,%a1
    8a3c:	346e fffe      	moveaw %fp@(-2),%a2
    8a40:	d3ca           	addal %a2,%a1
    8a42:	1280           	moveb %d0,%a1@
    8a44:	306e fffe      	moveaw %fp@(-2),%a0
    8a48:	226e 0008      	moveal %fp@(8),%a1
    8a4c:	1030 980e      	moveb %a0@(e,%a1:l),%d0
    8a50:	4880           	extw %d0
    8a52:	5340           	subqw #1,%d0
    8a54:	326e fffe      	moveaw %fp@(-2),%a1
    8a58:	d3c9           	addal %a1,%a1
    8a5a:	d3fc 0001 dc7c 	addal #121980,%a1
    8a60:	3280           	movew %d0,%a1@
    8a62:	302e fffe      	movew %fp@(-2),%d0
    8a66:	5240           	addqw #1,%d0
    8a68:	3d40 fffe      	movew %d0,%fp@(-2)
    8a6c:	0c6e 0010 fffe 	cmpiw #16,%fp@(-2)
    8a72:	6db2           	blts 0x8a26
    8a74:	4e5e           	unlk %fp
    8a76:	4e75           	rts
    8a78:	4e56 fff6      	linkw %fp,#-10
    8a7c:	426e fffe      	clrw %fp@(-2)
    8a80:	601e           	bras 0x8aa0
    8a82:	302e fffe      	movew %fp@(-2),%d0
    8a86:	4880           	extw %d0
    8a88:	227c 0004 549a 	moveal #283802,%a1
    8a8e:	346e fffe      	moveaw %fp@(-2),%a2
    8a92:	d3ca           	addal %a2,%a1
    8a94:	1280           	moveb %d0,%a1@
    8a96:	302e fffe      	movew %fp@(-2),%d0
    8a9a:	5240           	addqw #1,%d0
    8a9c:	3d40 fffe      	movew %d0,%fp@(-2)
    8aa0:	0c6e 0084 fffe 	cmpiw #132,%fp@(-2)
    8aa6:	6dda           	blts 0x8a82
    8aa8:	13fc 00ff 0004 	moveb #-1,0x4549b
    8aae:	549b 
    8ab0:	13fc 00ff 0004 	moveb #-1,0x4549d
    8ab6:	549d 
    8ab8:	13fc 00ff 0004 	moveb #-1,0x454a0
    8abe:	54a0 
    8ac0:	13fc 00ff 0004 	moveb #-1,0x454a2
    8ac6:	54a2 
    8ac8:	13fc 00ff 0004 	moveb #-1,0x454a4
    8ace:	54a4 
    8ad0:	0c6e 0001 0008 	cmpiw #1,%fp@(8)
    8ad6:	6700 0148      	beqw 0x8c20
    8ada:	0c6e 0009 0008 	cmpiw #9,%fp@(8)
    8ae0:	6c04           	bges 0x8ae6
    8ae2:	7001           	moveq #1,%d0
    8ae4:	6002           	bras 0x8ae8
    8ae6:	70ff           	moveq #-1,%d0
    8ae8:	1d40 fffa      	moveb %d0,%fp@(-6)
    8aec:	207c 0001 2116 	moveal #74006,%a0
    8af2:	326e 0008      	moveaw %fp@(8),%a1
    8af6:	d1c9           	addal %a1,%a0
    8af8:	1d50 fffc      	moveb %a0@,%fp@(-4)
    8afc:	426e fffe      	clrw %fp@(-2)
    8b00:	6000 0114      	braw 0x8c16
    8b04:	082e 0000 fffc 	btst #0,%fp@(-4)
    8b0a:	661e           	bnes 0x8b2a
    8b0c:	102e fffa      	moveb %fp@(-6),%d0
    8b10:	4880           	extw %d0
    8b12:	3f00           	movew %d0,%sp@-
    8b14:	207c 0004 549a 	moveal #283802,%a0
    8b1a:	326e fffe      	moveaw %fp@(-2),%a1
    8b1e:	d1c9           	addal %a1,%a0
    8b20:	1028 000b      	moveb %a0@(11),%d0
    8b24:	d05f           	addw %sp@+,%d0
    8b26:	1140 000b      	moveb %d0,%a0@(11)
    8b2a:	082e 0001 fffc 	btst #1,%fp@(-4)
    8b30:	661e           	bnes 0x8b50
    8b32:	102e fffa      	moveb %fp@(-6),%d0
    8b36:	4880           	extw %d0
    8b38:	3f00           	movew %d0,%sp@-
    8b3a:	207c 0004 549a 	moveal #283802,%a0
    8b40:	326e fffe      	moveaw %fp@(-2),%a1
    8b44:	d1c9           	addal %a1,%a0
    8b46:	1028 0009      	moveb %a0@(9),%d0
    8b4a:	d05f           	addw %sp@+,%d0
    8b4c:	1140 0009      	moveb %d0,%a0@(9)
    8b50:	082e 0002 fffc 	btst #2,%fp@(-4)
    8b56:	661e           	bnes 0x8b76
    8b58:	102e fffa      	moveb %fp@(-6),%d0
    8b5c:	4880           	extw %d0
    8b5e:	3f00           	movew %d0,%sp@-
    8b60:	207c 0004 549a 	moveal #283802,%a0
    8b66:	326e fffe      	moveaw %fp@(-2),%a1
    8b6a:	d1c9           	addal %a1,%a0
    8b6c:	1028 0007      	moveb %a0@(7),%d0
    8b70:	d05f           	addw %sp@+,%d0
    8b72:	1140 0007      	moveb %d0,%a0@(7)
    8b76:	082e 0003 fffc 	btst #3,%fp@(-4)
    8b7c:	661e           	bnes 0x8b9c
    8b7e:	102e fffa      	moveb %fp@(-6),%d0
    8b82:	4880           	extw %d0
    8b84:	3f00           	movew %d0,%sp@-
    8b86:	207c 0004 549a 	moveal #283802,%a0
    8b8c:	326e fffe      	moveaw %fp@(-2),%a1
    8b90:	d1c9           	addal %a1,%a0
    8b92:	1028 0005      	moveb %a0@(5),%d0
    8b96:	d05f           	addw %sp@+,%d0
    8b98:	1140 0005      	moveb %d0,%a0@(5)
    8b9c:	082e 0004 fffc 	btst #4,%fp@(-4)
    8ba2:	661e           	bnes 0x8bc2
    8ba4:	102e fffa      	moveb %fp@(-6),%d0
    8ba8:	4880           	extw %d0
    8baa:	3f00           	movew %d0,%sp@-
    8bac:	207c 0004 549a 	moveal #283802,%a0
    8bb2:	326e fffe      	moveaw %fp@(-2),%a1
    8bb6:	d1c9           	addal %a1,%a0
    8bb8:	1028 0004      	moveb %a0@(4),%d0
    8bbc:	d05f           	addw %sp@+,%d0
    8bbe:	1140 0004      	moveb %d0,%a0@(4)
    8bc2:	082e 0005 fffc 	btst #5,%fp@(-4)
    8bc8:	661e           	bnes 0x8be8
    8bca:	102e fffa      	moveb %fp@(-6),%d0
    8bce:	4880           	extw %d0
    8bd0:	3f00           	movew %d0,%sp@-
    8bd2:	207c 0004 549a 	moveal #283802,%a0
    8bd8:	326e fffe      	moveaw %fp@(-2),%a1
    8bdc:	d1c9           	addal %a1,%a0
    8bde:	1028 0002      	moveb %a0@(2),%d0
    8be2:	d05f           	addw %sp@+,%d0
    8be4:	1140 0002      	moveb %d0,%a0@(2)
    8be8:	082e 0006 fffc 	btst #6,%fp@(-4)
    8bee:	661a           	bnes 0x8c0a
    8bf0:	102e fffa      	moveb %fp@(-6),%d0
    8bf4:	4880           	extw %d0
    8bf6:	3f00           	movew %d0,%sp@-
    8bf8:	207c 0004 549a 	moveal #283802,%a0
    8bfe:	326e fffe      	moveaw %fp@(-2),%a1
    8c02:	d1c9           	addal %a1,%a0
    8c04:	1010           	moveb %a0@,%d0
    8c06:	d05f           	addw %sp@+,%d0
    8c08:	1080           	moveb %d0,%a0@
    8c0a:	302e fffe      	movew %fp@(-2),%d0
    8c0e:	d07c 000c      	addw #12,%d0
    8c12:	3d40 fffe      	movew %d0,%fp@(-2)
    8c16:	0c6e 0084 fffe 	cmpiw #132,%fp@(-2)
    8c1c:	6d00 fee6      	bltw 0x8b04
    8c20:	4e5e           	unlk %fp
    8c22:	4e75           	rts
    8c24:	4e56 fffa      	linkw %fp,#-6
    8c28:	306e 0008      	moveaw %fp@(8),%a0
    8c2c:	227c 0004 8bce 	moveal #297934,%a1
    8c32:	1030 9800      	moveb %a0@(0,%a1:l),%d0
    8c36:	4880           	extw %d0
    8c38:	c07c 000f      	andw #15,%d0
    8c3c:	3d40 fffe      	movew %d0,%fp@(-2)
    8c40:	306e fffe      	moveaw %fp@(-2),%a0
    8c44:	d1c8           	addal %a0,%a0
    8c46:	227c 0003 df96 	moveal #253846,%a1
    8c4c:	3030 9800      	movew %a0@(0,%a1:l),%d0
    8c50:	326e 0008      	moveaw %fp@(8),%a1
    8c54:	d3c9           	addal %a1,%a1
    8c56:	d3fc 0001 dc7c 	addal #121980,%a1
    8c5c:	3211           	movew %a1@,%d1
    8c5e:	b041           	cmpw %d1,%d0
    8c60:	6768           	beqs 0x8cca
    8c62:	4a79 0001 2126 	tstw 0x12126
    8c68:	6760           	beqs 0x8cca
    8c6a:	306e 0008      	moveaw %fp@(8),%a0
    8c6e:	227c 0004 8bce 	moveal #297934,%a1
    8c74:	1030 9800      	moveb %a0@(0,%a1:l),%d0
    8c78:	4880           	extw %d0
    8c7a:	c07c 000f      	andw #15,%d0
    8c7e:	807c 00c0      	orw #192,%d0
    8c82:	13c0 0004 93f2 	moveb %d0,0x493f2
    8c88:	306e 0008      	moveaw %fp@(8),%a0
    8c8c:	d1c8           	addal %a0,%a0
    8c8e:	227c 0001 dc7c 	moveal #121980,%a1
    8c94:	3030 9800      	movew %a0@(0,%a1:l),%d0
    8c98:	4880           	extw %d0
    8c9a:	13c0 0004 93f3 	moveb %d0,0x493f3
    8ca0:	306e fffe      	moveaw %fp@(-2),%a0
    8ca4:	d1c8           	addal %a0,%a0
    8ca6:	d1fc 0003 df96 	addal #253846,%a0
    8cac:	326e 0008      	moveaw %fp@(8),%a1
    8cb0:	d3c9           	addal %a1,%a1
    8cb2:	d3fc 0001 dc7c 	addal #121980,%a1
    8cb8:	3091           	movew %a1@,%a0@
    8cba:	4257           	clrw %sp@
    8cbc:	3f3c 0002      	movew #2,%sp@-
    8cc0:	2f3c 0004 93f2 	movel #300018,%sp@-
    8cc6:	6106           	bsrs 0x8cce
    8cc8:	5c8f           	addql #6,%sp
    8cca:	4e5e           	unlk %fp
    8ccc:	4e75           	rts
    8cce:	4e56 ffd0      	linkw %fp,#-48
    8cd2:	2d6e 0008 fffc 	movel %fp@(8),%fp@(-4)
    8cd8:	4a79 0001 2126 	tstw 0x12126
    8cde:	6700 00a6      	beqw 0x8d86
    8ce2:	206e 0008      	moveal %fp@(8),%a0
    8ce6:	1d68 0001 fffa 	moveb %a0@(1),%fp@(-6)
    8cec:	4a6e 000e      	tstw %fp@(14)
    8cf0:	6732           	beqs 0x8d24
    8cf2:	3039 0001 2130 	movew 0x12130,%d0
    8cf8:	322e 000e      	movew %fp@(14),%d1
    8cfc:	e841           	asrw #4,%d1
    8cfe:	c27c 000f      	andw #15,%d1
    8d02:	9041           	subw %d1,%d0
    8d04:	3d40 ffe0      	movew %d0,%fp@(-32)
    8d08:	206e 0008      	moveal %fp@(8),%a0
    8d0c:	1028 0001      	moveb %a0@(1),%d0
    8d10:	4880           	extw %d0
    8d12:	322e ffe0      	movew %fp@(-32),%d1
    8d16:	c3fc fff4      	mulsw #-12,%d1
    8d1a:	d041           	addw %d1,%d0
    8d1c:	226e 0008      	moveal %fp@(8),%a1
    8d20:	1340 0001      	moveb %d0,%a1@(1)
    8d24:	0c79 0001 0001 	cmpiw #1,0x12108
    8d2a:	2108 
    8d2c:	662e           	bnes 0x8d5c
    8d2e:	6024           	bras 0x8d54
    8d30:	206e 0008      	moveal %fp@(8),%a0
    8d34:	1010           	moveb %a0@,%d0
    8d36:	4880           	extw %d0
    8d38:	3e80           	movew %d0,%sp@
    8d3a:	4eb9 0000 9692 	jsr 0x9692
    8d40:	202e 0008      	movel %fp@(8),%d0
    8d44:	5280           	addql #1,%d0
    8d46:	2d40 0008      	movel %d0,%fp@(8)
    8d4a:	302e 000c      	movew %fp@(12),%d0
    8d4e:	5340           	subqw #1,%d0
    8d50:	3d40 000c      	movew %d0,%fp@(12)
    8d54:	4a6e 000c      	tstw %fp@(12)
    8d58:	66d6           	bnes 0x8d30
    8d5a:	6020           	bras 0x8d7c
    8d5c:	4297           	clrl %sp@
    8d5e:	2f2e 0008      	movel %fp@(8),%sp@-
    8d62:	302e 000c      	movew %fp@(12),%d0
    8d66:	5340           	subqw #1,%d0
    8d68:	48c0           	extl %d0
    8d6a:	2f00           	movel %d0,%sp@-
    8d6c:	3f3c 000c      	movew #12,%sp@-
    8d70:	4eb9 0000 ead2 	jsr 0xead2
    8d76:	dffc 0000 000a 	addal #10,%sp
    8d7c:	206e fffc      	moveal %fp@(-4),%a0
    8d80:	116e fffa 0001 	moveb %fp@(-6),%a0@(1)
    8d86:	4a79 0001 212c 	tstw 0x1212c
    8d8c:	6606           	bnes 0x8d94
    8d8e:	7001           	moveq #1,%d0
    8d90:	6000 04ec      	braw 0x927e
    8d94:	202e fffc      	movel %fp@(-4),%d0
    8d98:	5280           	addql #1,%d0
    8d9a:	2d40 fff6      	movel %d0,%fp@(-10)
    8d9e:	206e fffc      	moveal %fp@(-4),%a0
    8da2:	1010           	moveb %a0@,%d0
    8da4:	4880           	extw %d0
    8da6:	c07c 00f0      	andw #240,%d0
    8daa:	b07c 0090      	cmpw #144,%d0
    8dae:	6706           	beqs 0x8db6
    8db0:	4240           	clrw %d0
    8db2:	6000 04ca      	braw 0x927e
    8db6:	206e fffc      	moveal %fp@(-4),%a0
    8dba:	4a28 0002      	tstb %a0@(2)
    8dbe:	6600 0080      	bnew 0x8e40
    8dc2:	426e fff4      	clrw %fp@(-12)
    8dc6:	6026           	bras 0x8dee
    8dc8:	306e fff4      	moveaw %fp@(-12),%a0
    8dcc:	227c 0004 9596 	moveal #300438,%a1
    8dd2:	1030 9800      	moveb %a0@(0,%a1:l),%d0
    8dd6:	4880           	extw %d0
    8dd8:	226e fff6      	moveal %fp@(-10),%a1
    8ddc:	1211           	moveb %a1@,%d1
    8dde:	4881           	extw %d1
    8de0:	b041           	cmpw %d1,%d0
    8de2:	6712           	beqs 0x8df6
    8de4:	302e fff4      	movew %fp@(-12),%d0
    8de8:	5240           	addqw #1,%d0
    8dea:	3d40 fff4      	movew %d0,%fp@(-12)
    8dee:	0c6e 0003 fff4 	cmpiw #3,%fp@(-12)
    8df4:	6dd2           	blts 0x8dc8
    8df6:	0c6e 0003 fff4 	cmpiw #3,%fp@(-12)
    8dfc:	6d06           	blts 0x8e04
    8dfe:	4240           	clrw %d0
    8e00:	6000 047c      	braw 0x927e
    8e04:	207c 0004 9596 	moveal #300438,%a0
    8e0a:	326e fff4      	moveaw %fp@(-12),%a1
    8e0e:	d1c9           	addal %a1,%a0
    8e10:	4210           	clrb %a0@
    8e12:	302e fff4      	movew %fp@(-12),%d0
    8e16:	c1fc 000a      	mulsw #10,%d0
    8e1a:	d0bc 0004 8ed4 	addl #298708,%d0
    8e20:	2040           	moveal %d0,%a0
    8e22:	10bc 0004      	moveb #4,%a0@
    8e26:	302e fff4      	movew %fp@(-12),%d0
    8e2a:	c1fc 000a      	mulsw #10,%d0
    8e2e:	d0bc 0004 8ed4 	addl #298708,%d0
    8e34:	2040           	moveal %d0,%a0
    8e36:	4228 0001      	clrb %a0@(1)
    8e3a:	7001           	moveq #1,%d0
    8e3c:	6000 0440      	braw 0x927e
    8e40:	426e fff2      	clrw %fp@(-14)
    8e44:	601a           	bras 0x8e60
    8e46:	207c 0004 9596 	moveal #300438,%a0
    8e4c:	326e fff2      	moveaw %fp@(-14),%a1
    8e50:	d1c9           	addal %a1,%a0
    8e52:	4a10           	tstb %a0@
    8e54:	6712           	beqs 0x8e68
    8e56:	302e fff2      	movew %fp@(-14),%d0
    8e5a:	5240           	addqw #1,%d0
    8e5c:	3d40 fff2      	movew %d0,%fp@(-14)
    8e60:	0c6e 0003 fff2 	cmpiw #3,%fp@(-14)
    8e66:	6dde           	blts 0x8e46
    8e68:	0c6e 0003 fff2 	cmpiw #3,%fp@(-14)
    8e6e:	6654           	bnes 0x8ec4
    8e70:	426e fff2      	clrw %fp@(-14)
    8e74:	3d7c 0001 fff0 	movew #1,%fp@(-16)
    8e7a:	6040           	bras 0x8ebc
    8e7c:	302e fff0      	movew %fp@(-16),%d0
    8e80:	5340           	subqw #1,%d0
    8e82:	c1fc 000a      	mulsw #10,%d0
    8e86:	2040           	moveal %d0,%a0
    8e88:	227c 0004 8ed4 	moveal #298708,%a1
    8e8e:	1030 9800      	moveb %a0@(0,%a1:l),%d0
    8e92:	4880           	extw %d0
    8e94:	322e fff0      	movew %fp@(-16),%d1
    8e98:	c3fc 000a      	mulsw #10,%d1
    8e9c:	d2bc 0004 8ed4 	addl #298708,%d1
    8ea2:	2241           	moveal %d1,%a1
    8ea4:	1211           	moveb %a1@,%d1
    8ea6:	4881           	extw %d1
    8ea8:	b041           	cmpw %d1,%d0
    8eaa:	6c06           	bges 0x8eb2
    8eac:	3d6e fff0 fff2 	movew %fp@(-16),%fp@(-14)
    8eb2:	302e fff0      	movew %fp@(-16),%d0
    8eb6:	5240           	addqw #1,%d0
    8eb8:	3d40 fff0      	movew %d0,%fp@(-16)
    8ebc:	0c6e 0003 fff0 	cmpiw #3,%fp@(-16)
    8ec2:	6db8           	blts 0x8e7c
    8ec4:	206e fff6      	moveal %fp@(-10),%a0
    8ec8:	1010           	moveb %a0@,%d0
    8eca:	4880           	extw %d0
    8ecc:	b039 0001 2132 	cmpb 0x12132,%d0
    8ed2:	6d10           	blts 0x8ee4
    8ed4:	206e fff6      	moveal %fp@(-10),%a0
    8ed8:	1010           	moveb %a0@,%d0
    8eda:	4880           	extw %d0
    8edc:	b039 0001 2134 	cmpb 0x12134,%d0
    8ee2:	6f06           	bles 0x8eea
    8ee4:	7001           	moveq #1,%d0
    8ee6:	6000 0396      	braw 0x927e
    8eea:	3d7c 0001 ffee 	movew #1,%fp@(-18)
    8ef0:	3ebc 0008      	movew #8,%sp@
    8ef4:	302e fff2      	movew %fp@(-14),%d0
    8ef8:	c1fc 000a      	mulsw #10,%d0
    8efc:	d0bc 0004 8ed4 	addl #298708,%d0
    8f02:	2f00           	movel %d0,%sp@-
    8f04:	5497           	addql #2,%sp@
    8f06:	3039 0001 2136 	movew 0x12136,%d0
    8f0c:	5340           	subqw #1,%d0
    8f0e:	48c0           	extl %d0
    8f10:	e780           	asll #3,%d0
    8f12:	d0b9 0001 20ee 	addl 0x120ee,%d0
    8f18:	2f00           	movel %d0,%sp@-
    8f1a:	4eb9 0000 96b2 	jsr 0x96b2
    8f20:	508f           	addql #8,%sp
    8f22:	302e fff2      	movew %fp@(-14),%d0
    8f26:	c1fc 000a      	mulsw #10,%d0
    8f2a:	d0bc 0004 8ed4 	addl #298708,%d0
    8f30:	2040           	moveal %d0,%a0
    8f32:	1d68 0002 ffea 	moveb %a0@(2),%fp@(-22)
    8f38:	302e fff2      	movew %fp@(-14),%d0
    8f3c:	c1fc 000a      	mulsw #10,%d0
    8f40:	2040           	moveal %d0,%a0
    8f42:	227c 0004 8ed4 	moveal #298708,%a1
    8f48:	1030 9802      	moveb %a0@(2,%a1:l),%d0
    8f4c:	4880           	extw %d0
    8f4e:	c07c 000f      	andw #15,%d0
    8f52:	322e fff2      	movew %fp@(-14),%d1
    8f56:	c3fc 000a      	mulsw #10,%d1
    8f5a:	d2bc 0004 8ed4 	addl #298708,%d1
    8f60:	2241           	moveal %d1,%a1
    8f62:	1340 0002      	moveb %d0,%a1@(2)
    8f66:	7002           	moveq #2,%d0
    8f68:	322e fff2      	movew %fp@(-14),%d1
    8f6c:	c3fc 000a      	mulsw #10,%d1
    8f70:	d2bc 0004 8ed4 	addl #298708,%d1
    8f76:	2241           	moveal %d1,%a1
    8f78:	1229 0003      	moveb %a1@(3),%d1
    8f7c:	4881           	extw %d1
    8f7e:	e841           	asrw #4,%d1
    8f80:	c27c 000f      	andw #15,%d1
    8f84:	9041           	subw %d1,%d0
    8f86:	c1fc 000c      	mulsw #12,%d0
    8f8a:	4880           	extw %d0
    8f8c:	1d40 ffec      	moveb %d0,%fp@(-20)
    8f90:	302e fff2      	movew %fp@(-14),%d0
    8f94:	c1fc 000a      	mulsw #10,%d0
    8f98:	2040           	moveal %d0,%a0
    8f9a:	227c 0004 8ed4 	moveal #298708,%a1
    8fa0:	1030 9803      	moveb %a0@(3,%a1:l),%d0
    8fa4:	4880           	extw %d0
    8fa6:	c07c 000f      	andw #15,%d0
    8faa:	322e fff2      	movew %fp@(-14),%d1
    8fae:	c3fc 000a      	mulsw #10,%d1
    8fb2:	d2bc 0004 8ed4 	addl #298708,%d1
    8fb8:	2241           	moveal %d1,%a1
    8fba:	1340 0003      	moveb %d0,%a1@(3)
    8fbe:	102e ffea      	moveb %fp@(-22),%d0
    8fc2:	4880           	extw %d0
    8fc4:	e840           	asrw #4,%d0
    8fc6:	c07c 000f      	andw #15,%d0
    8fca:	322e fff2      	movew %fp@(-14),%d1
    8fce:	e360           	aslw %d1,%d0
    8fd0:	3d40 ffde      	movew %d0,%fp@(-34)
    8fd4:	7009           	moveq #9,%d0
    8fd6:	322e fff2      	movew %fp@(-14),%d1
    8fda:	e360           	aslw %d1,%d0
    8fdc:	4640           	notw %d0
    8fde:	3d40 ffdc      	movew %d0,%fp@(-36)
    8fe2:	206e fff6      	moveal %fp@(-10),%a0
    8fe6:	1010           	moveb %a0@,%d0
    8fe8:	4880           	extw %d0
    8fea:	122e ffec      	moveb %fp@(-20),%d1
    8fee:	4881           	extw %d1
    8ff0:	d041           	addw %d1,%d0
    8ff2:	3d40 ffe4      	movew %d0,%fp@(-28)
    8ff6:	0c79 0001 0001 	cmpiw #1,0x12108
    8ffc:	2108 
    8ffe:	6662           	bnes 0x9062
    9000:	7006           	moveq #6,%d0
    9002:	4880           	extw %d0
    9004:	3e80           	movew %d0,%sp@
    9006:	4240           	clrw %d0
    9008:	302e ffe4      	movew %fp@(-28),%d0
    900c:	e348           	lslw #1,%d0
    900e:	4840           	swap %d0
    9010:	4240           	clrw %d0
    9012:	4840           	swap %d0
    9014:	2040           	moveal %d0,%a0
    9016:	227c 0001 2a76 	moveal #76406,%a1
    901c:	4240           	clrw %d0
    901e:	3030 9800      	movew %a0@(0,%a1:l),%d0
    9022:	4840           	swap %d0
    9024:	4240           	clrw %d0
    9026:	4840           	swap %d0
    9028:	80fc 003c      	divuw #60,%d0
    902c:	c07c 00ff      	andw #255,%d0
    9030:	3f00           	movew %d0,%sp@-
    9032:	4eb9 0000 96ea 	jsr 0x96ea
    9038:	548f           	addql #2,%sp
    903a:	4240           	clrw %d0
    903c:	302e ffdc      	movew %fp@(-36),%d0
    9040:	c07c 00ff      	andw #255,%d0
    9044:	3e80           	movew %d0,%sp@
    9046:	0057 00c0      	oriw #192,%sp@
    904a:	4240           	clrw %d0
    904c:	302e ffde      	movew %fp@(-34),%d0
    9050:	c07c 00ff      	andw #255,%d0
    9054:	3f00           	movew %d0,%sp@-
    9056:	4eb9 0000 970e 	jsr 0x970e
    905c:	548f           	addql #2,%sp
    905e:	6000 00ac      	braw 0x910c
    9062:	4297           	clrl %sp@
    9064:	2f3c 0000 0086 	movel #134,%sp@-
    906a:	4240           	clrw %d0
    906c:	302e ffe4      	movew %fp@(-28),%d0
    9070:	e348           	lslw #1,%d0
    9072:	4840           	swap %d0
    9074:	4240           	clrw %d0
    9076:	4840           	swap %d0
    9078:	2040           	moveal %d0,%a0
    907a:	227c 0001 2a76 	moveal #76406,%a1
    9080:	4240           	clrw %d0
    9082:	3030 9800      	movew %a0@(0,%a1:l),%d0
    9086:	4840           	swap %d0
    9088:	4240           	clrw %d0
    908a:	4840           	swap %d0
    908c:	80fc 003c      	divuw #60,%d0
    9090:	4840           	swap %d0
    9092:	4240           	clrw %d0
    9094:	4840           	swap %d0
    9096:	2f00           	movel %d0,%sp@-
    9098:	3f3c 001c      	movew #28,%sp@-
    909c:	4eb9 0000 ead2 	jsr 0xead2
    90a2:	dffc 0000 000a 	addal #10,%sp
    90a8:	4297           	clrl %sp@
    90aa:	2f3c 0000 0007 	movel #7,%sp@-
    90b0:	42a7           	clrl %sp@-
    90b2:	3f3c 001c      	movew #28,%sp@-
    90b6:	4eb9 0000 ead2 	jsr 0xead2
    90bc:	dffc 0000 000a 	addal #10,%sp
    90c2:	2d40 ffd8      	movel %d0,%fp@(-40)
    90c6:	4280           	clrl %d0
    90c8:	302e ffde      	movew %fp@(-34),%d0
    90cc:	4241           	clrw %d1
    90ce:	322e ffdc      	movew %fp@(-36),%d1
    90d2:	827c 00c0      	orw #192,%d1
    90d6:	4841           	swap %d1
    90d8:	4241           	clrw %d1
    90da:	4841           	swap %d1
    90dc:	c2ae ffd8      	andl %fp@(-40),%d1
    90e0:	8081           	orl %d1,%d0
    90e2:	2d40 ffd4      	movel %d0,%fp@(-44)
    90e6:	4297           	clrl %sp@
    90e8:	202e ffd4      	movel %fp@(-44),%d0
    90ec:	48c0           	extl %d0
    90ee:	2f00           	movel %d0,%sp@-
    90f0:	202e ffd4      	movel %fp@(-44),%d0
    90f4:	7210           	moveq #16,%d1
    90f6:	e2a0           	asrl %d1,%d0
    90f8:	48c0           	extl %d0
    90fa:	2f00           	movel %d0,%sp@-
    90fc:	3f3c 001c      	movew #28,%sp@-
    9100:	4eb9 0000 ead2 	jsr 0xead2
    9106:	dffc 0000 000a 	addal #10,%sp
    910c:	302e fff2      	movew %fp@(-14),%d0
    9110:	e340           	aslw #1,%d0
    9112:	3d40 ffe2      	movew %d0,%fp@(-30)
    9116:	206e fff6      	moveal %fp@(-10),%a0
    911a:	1010           	moveb %a0@,%d0
    911c:	4880           	extw %d0
    911e:	122e ffec      	moveb %fp@(-20),%d1
    9122:	4881           	extw %d1
    9124:	d041           	addw %d1,%d0
    9126:	b07c 0017      	cmpw #23,%d0
    912a:	6c0a           	bges 0x9136
    912c:	3d7c 0005 ffee 	movew #5,%fp@(-18)
    9132:	6000 00c8      	braw 0x91fc
    9136:	4240           	clrw %d0
    9138:	302e ffe4      	movew %fp@(-28),%d0
    913c:	e348           	lslw #1,%d0
    913e:	4840           	swap %d0
    9140:	4240           	clrw %d0
    9142:	4840           	swap %d0
    9144:	d0bc 0001 2a76 	addl #76406,%d0
    914a:	2040           	moveal %d0,%a0
    914c:	3d50 ffe8      	movew %a0@,%fp@(-24)
    9150:	4240           	clrw %d0
    9152:	302e ffe8      	movew %fp@(-24),%d0
    9156:	e048           	lsrw #8,%d0
    9158:	c07c 000f      	andw #15,%d0
    915c:	3d40 ffe6      	movew %d0,%fp@(-26)
    9160:	0c79 0001 0001 	cmpiw #1,0x12108
    9166:	2108 
    9168:	663c           	bnes 0x91a6
    916a:	302e ffe2      	movew %fp@(-30),%d0
    916e:	4880           	extw %d0
    9170:	3e80           	movew %d0,%sp@
    9172:	4240           	clrw %d0
    9174:	302e ffe8      	movew %fp@(-24),%d0
    9178:	c07c 00ff      	andw #255,%d0
    917c:	3f00           	movew %d0,%sp@-
    917e:	4eb9 0000 96ea 	jsr 0x96ea
    9184:	548f           	addql #2,%sp
    9186:	302e ffe2      	movew %fp@(-30),%d0
    918a:	5240           	addqw #1,%d0
    918c:	4880           	extw %d0
    918e:	3e80           	movew %d0,%sp@
    9190:	4240           	clrw %d0
    9192:	302e ffe6      	movew %fp@(-26),%d0
    9196:	c07c 00ff      	andw #255,%d0
    919a:	3f00           	movew %d0,%sp@-
    919c:	4eb9 0000 96ea 	jsr 0x96ea
    91a2:	548f           	addql #2,%sp
    91a4:	6056           	bras 0x91fc
    91a6:	4297           	clrl %sp@
    91a8:	302e ffe2      	movew %fp@(-30),%d0
    91ac:	d07c 0080      	addw #128,%d0
    91b0:	48c0           	extl %d0
    91b2:	2f00           	movel %d0,%sp@-
    91b4:	4240           	clrw %d0
    91b6:	302e ffe8      	movew %fp@(-24),%d0
    91ba:	c07c 00ff      	andw #255,%d0
    91be:	4840           	swap %d0
    91c0:	4240           	clrw %d0
    91c2:	4840           	swap %d0
    91c4:	2f00           	movel %d0,%sp@-
    91c6:	3f3c 001c      	movew #28,%sp@-
    91ca:	4eb9 0000 ead2 	jsr 0xead2
    91d0:	dffc 0000 000a 	addal #10,%sp
    91d6:	4297           	clrl %sp@
    91d8:	302e ffe2      	movew %fp@(-30),%d0
    91dc:	d07c 0081      	addw #129,%d0
    91e0:	48c0           	extl %d0
    91e2:	2f00           	movel %d0,%sp@-
    91e4:	4280           	clrl %d0
    91e6:	302e ffe6      	movew %fp@(-26),%d0
    91ea:	2f00           	movel %d0,%sp@-
    91ec:	3f3c 001c      	movew #28,%sp@-
    91f0:	4eb9 0000 ead2 	jsr 0xead2
    91f6:	dffc 0000 000a 	addal #10,%sp
    91fc:	207c 0004 9596 	moveal #300438,%a0
    9202:	326e fff2      	moveaw %fp@(-14),%a1
    9206:	d1c9           	addal %a1,%a0
    9208:	226e fff6      	moveal %fp@(-10),%a1
    920c:	1091           	moveb %a1@,%a0@
    920e:	0c6e 0005 ffee 	cmpiw #5,%fp@(-18)
    9214:	6614           	bnes 0x922a
    9216:	302e fff2      	movew %fp@(-14),%d0
    921a:	c1fc 000a      	mulsw #10,%d0
    921e:	d0bc 0004 8ed4 	addl #298708,%d0
    9224:	2040           	moveal %d0,%a0
    9226:	4228 0007      	clrb %a0@(7)
    922a:	3039 0001 20f6 	movew 0x120f6,%d0
    9230:	4880           	extw %d0
    9232:	322e fff2      	movew %fp@(-14),%d1
    9236:	c3fc 000a      	mulsw #10,%d1
    923a:	d2bc 0004 8ed4 	addl #298708,%d1
    9240:	2241           	moveal %d1,%a1
    9242:	1340 0008      	moveb %d0,%a1@(8)
    9246:	302e fff2      	movew %fp@(-14),%d0
    924a:	c1fc 000a      	mulsw #10,%d0
    924e:	d0bc 0004 8ed4 	addl #298708,%d0
    9254:	2040           	moveal %d0,%a0
    9256:	117c 0001 0001 	moveb #1,%a0@(1)
    925c:	33fc 0001 0001 	movew #1,0x1212e
    9262:	212e 
    9264:	302e ffee      	movew %fp@(-18),%d0
    9268:	4880           	extw %d0
    926a:	322e fff2      	movew %fp@(-14),%d1
    926e:	c3fc 000a      	mulsw #10,%d1
    9272:	d2bc 0004 8ed4 	addl #298708,%d1
    9278:	2241           	moveal %d1,%a1
    927a:	1280           	moveb %d0,%a1@
    927c:	7001           	moveq #1,%d0
    927e:	4e5e           	unlk %fp
    9280:	4e75           	rts
    9282:	Address 0x9284 is out of bounds.
Address 0x9284 is out of bounds.


* ================= RECONSTRUCTION NOTES (2026-09-01) =================
* Region 0x86ee-0x9282 = the FAITHFUL midi_seq.o remainder.
* Layout:
*   0x86ee  static: ev_pgm-like  (byte[2] -> $120fe; bsr mq_bust; p+3)
*   0x8716  static: ev_tempo     (byte[1] -> $12102; 2400/x -> $12100; p+2)
*   0x8746  static: {return p+2}
*   0x8758  static: {return p+3}
*   0x876a  static: {return p+3}
*   0x8778  mq_inis..mq_sepc     (nine functions, ALREADY byte-matched)
*   0x8cce  THE ENGINE (one fn, link #-48, 1460 B):
*     args: (event_ptr.l @8, count.w @12, transpose_nibbles.w @14)
*     - if ($12126) transpose note byte[1] by octaves from $12130
*     - if ($12108==1) send raw bytes via psg path (jsr 0x9692 per
*       byte) else Midiws(count-1, ptr)  [xbios(12,n,b,0L) padded]
*     - only 0x9x (note-on) status handled further; 3-voice alloc:
*       channel-note table $49596[3], voice structs $48ed4[3] stride
*       10; voice stealing by lowest byte[0]; note range clamp
*       [$12132..$12134]; envelope copy via 0x96b2(psg_cpen?) from
*       $120ee + (($12136-1)<<3); Giaccess writes = PADDED
*       xbios(0x1C, val, reg, 0L) incl. mixer reg 7 read/modify;
*       freq table $12a76[note<<1] / 60 -> psg regs via 0x96ea
*       (psg-write) or MIDI path via xbios(0x1C,...) pairs; drum
*       branch when note+transp < 23 -> voice flag 5, clrb byte[7];
*       byte[8] = $120f6; byte[1]=1; $1212e=1; byte[0]=voice-state.
*     Helpers 0x9692/0x96b2/0x96ea/0x970e are psg_io.o == port's
*     matched psg_io functions -- reuse them by port name.
* For the FAITHFUL build, gate the kept engine out of midi_seq.c and
* implement the five statics + this engine; the nine matched mq_*
* functions stay shared.
* =====================================================================
