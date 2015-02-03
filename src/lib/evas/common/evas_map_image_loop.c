
#ifdef SCALE_USING_MMX
#undef SCALE_USING_MMX
#endif

#ifdef SMOOTH
{
# ifdef SCALE_USING_MMX
#  ifdef COLMUL
#   ifdef COLSAME
   MOV_P2R(c1, mm7, mm0); // col
#   endif //COLSAME
#  endif //COLMUL
# endif //SCALE_USING_MMX

# ifdef SCALE_USING_NEON
<<<<<<< HEAD
#  ifndef COLBLACK
   uint16x4_t temp_16x4;
   uint16x4_t rv_16x4;
   uint16x4_t val1_16x4;
   uint16x4_t val3_16x4;
   uint16x8_t ru_16x8;
   uint16x8_t val1_val3_16x8;
   uint16x8_t val2_val4_16x8;
   uint16x8_t x255_16x8;
   uint32x2_t res_32x2;
   uint32x2_t val1_val3_32x2;
   uint32x2_t val2_val4_32x2;
   uint8x8_t val1_val3_8x8;
   uint8x8_t val2_val4_8x8;

   x255_16x8 = vdupq_n_u16(0xff);
#   ifdef COLMUL
   uint16x4_t x255_16x4;
   x255_16x4 = vget_low_u16(x255_16x8);
   uint16x4_t c1_16x4;
#    ifdef COLSAME
   uint16x4_t c1_val3_16x4;
   uint16x8_t c1_16x8;
   uint16x8_t c1_val3_16x8;
   uint32x2_t c1_32x2;
   uint8x8_t c1_8x8;
   uint8x8_t c1_val3_8x8;

   c1_32x2 = vset_lane_u32(c1, c1_32x2, 0);
   c1_8x8 = vreinterpret_u8_u32(c1_32x2);
   c1_16x8 = vmovl_u8(c1_8x8);
   c1_16x4 = vget_low_u16(c1_16x8);
#    else //COLSAME
   uint16x4_t c2_16x4;
   uint16x4_t c2_local_16x4;
   uint16x4_t cv_16x4;
   uint16x8_t c1_c2_16x8;
   uint16x8_t c1_val1_16x8;
   uint16x8_t c2_val3_16x8;
   uint16x8_t cv_rv_16x8;
   uint32x2_t c1_c2_32x2;
   uint8x8_t c1_c2_8x8;
   uint8x8_t val3_8x8;
   uint16x8_t val3_16x8;

   c1_c2_32x2 = vset_lane_u32(c1, c1_c2_32x2, 0);
   c1_c2_32x2 = vset_lane_u32(c2, c1_c2_32x2, 1);
   c1_c2_8x8 = vreinterpret_u8_u32(c1_c2_32x2);
   c1_c2_16x8 = vmovl_u8(c1_c2_8x8);
   c1_16x4 = vget_low_u16(c1_c2_16x8);
   c2_16x4 = vget_high_u16(c1_c2_16x8);
#    endif //COLSAME
#   else //COLMUL
   uint8x8_t val3_8x8;
   uint16x8_t val3_16x8;
#   endif //COLMUL
#  endif //COLBLACK
=======
   FPU_NEON;
   VMOV_I2R_NEON(q2, #255);
#  ifdef COLMUL
#   ifndef COLBLACK
   // this part can be done here as c1 and c2 are constants in the cycle
   FPU_NEON;
   VMOV_M2R_NEON(d18, c1);
   VEOR_NEON(q8);
#    ifndef COLSAME
   VMOV_M2R_NEON(d19, c2);
#    endif //COLSAME
   VZIP_NEON(q9, q8);
#    ifndef COLSAME
   VMOV_R2R_NEON(d19, d16);
#    endif //COLSAME
   // here we have c1 and c2 spread through q9 register
#   endif //COLBLACK
#  endif //COLMUL
>>>>>>> 1c816daeed970349f31eb94763657ae1637dbfda
# endif //SCALE_USING_NEON

   while (ww > 0)
     {
        DATA32 val1 = 0x00000000;
# ifdef COLBLACK
        *d = 0xff000000; // col
# else  //COLBLACK
        FPc uu1, vv1, uu2, vv2;
        FPc rv, ru;
        DATA32 val2, val3, val4;

        uu1 = u;
        if (uu1 < 0) uu1 = 0;
        else if (uu1 >= swp) uu1 = swp - 1;

        vv1 = v;
        if (vv1 < 0) vv1 = 0;
        else if (vv1 >= shp) vv1 = shp - 1;

        uu2 = uu1 + FPFPI1;      // next u point
        if (uu2 >= swp) uu2 = swp - 1;

        vv2 = vv1 + FPFPI1;      // next v point
        if (vv2 >= shp) vv2 = shp - 1;

        ru = (u >> (FP + FPI - 8)) & 0xff;
        rv = (v >> (FP + FPI - 8)) & 0xff;

        s = sp + ((vv1 >> (FP + FPI)) * sw) + (uu1 >> (FP + FPI));
        val1 = *s;             // current pixel

        s = sp + ((vv1 >> (FP + FPI)) * sw) + (uu2 >> (FP + FPI));
        val2 = *s;             // right pixel

        s = sp + ((vv2 >> (FP + FPI)) * sw) + (uu1 >> (FP + FPI));
        val3 = *s;             // bottom pixel

        s = sp + ((vv2 >> (FP + FPI)) * sw) + (uu2 >> (FP + FPI));
        val4 = *s;             // right bottom pixel

#  ifdef SCALE_USING_MMX
        MOV_A2R(rv, mm4);
        MOV_A2R(ru, mm6);
        MOV_P2R(val1, mm1, mm0);
        if (val1 | val2)
          {
             MOV_P2R(val2, mm2, mm0);
             INTERP_256_R2R(mm6, mm2, mm1, mm5);
          }
        MOV_P2R(val3, mm2, mm0);
        if (val3 | val4)
          {
             MOV_P2R(val4, mm3, mm0);
             INTERP_256_R2R(mm6, mm3, mm2, mm5);
          }
        INTERP_256_R2R(mm4, mm2, mm1, mm5);
#   ifdef COLMUL
#    ifdef COLSAME
//        MOV_P2R(c1, mm7, mm0); // col
        MUL4_SYM_R2R(mm7, mm1, mm5); // col
#    else //COLSAME
        cc = cv >> 16; // col
        cv += cd; // col
        MOV_A2R(cc, mm2); // col
        MOV_P2R(c1, mm3, mm0); // col
        MOV_P2R(c2, mm4, mm0); // col
        INTERP_256_R2R(mm2, mm4, mm3, mm5); // col
        MUL4_SYM_R2R(mm3, mm1, mm5); // col
#    endif //COLSAME
#   endif //COLMUL
        MOV_R2P(mm1, *d, mm0);
#  elif defined SCALE_USING_NEON
        if (val1 | val2 | val3 | val4)
          {
<<<<<<< HEAD
             rv_16x4 = vdup_n_u16(rv);
             ru_16x8 = vdupq_n_u16(ru);

             val1_val3_32x2 = vset_lane_u32(val1, val1_val3_32x2, 0);
             val1_val3_32x2 = vset_lane_u32(val3, val1_val3_32x2, 1);
             val2_val4_32x2 = vset_lane_u32(val2, val2_val4_32x2, 0);
             val2_val4_32x2 = vset_lane_u32(val4, val2_val4_32x2, 1);

             val1_val3_8x8 = vreinterpret_u8_u32(val1_val3_32x2);
             val2_val4_8x8 = vreinterpret_u8_u32(val2_val4_32x2);

             val2_val4_16x8 = vmovl_u8(val2_val4_8x8);
             val1_val3_16x8 = vmovl_u8(val1_val3_8x8);

             val2_val4_16x8 = vsubq_u16(val2_val4_16x8, val1_val3_16x8);
             val2_val4_16x8 = vmulq_u16(val2_val4_16x8, ru_16x8);
             val2_val4_16x8 = vshrq_n_u16(val2_val4_16x8, 8);
             val2_val4_16x8 = vaddq_u16(val2_val4_16x8, val1_val3_16x8);
             val2_val4_16x8 = vandq_u16(val2_val4_16x8, x255_16x8);

             val1_16x4 = vget_low_u16(val2_val4_16x8);
             val3_16x4 = vget_high_u16(val2_val4_16x8);
#   ifdef COLMUL
#    ifdef COLSAME

             val3_16x4 = vsub_u16(val3_16x4, val1_16x4);
             val3_16x4 = vmul_u16(val3_16x4, rv_16x4);
             val3_16x4 = vshr_n_u16(val3_16x4, 8);
             val3_16x4 = vadd_u16(val3_16x4, val1_16x4);
             val3_16x4 = vand_u16(val3_16x4, x255_16x4);

             c1_val3_16x4 = vmul_u16(c1_16x4, val3_16x4);
             c1_val3_16x4 = vadd_u16(c1_val3_16x4, x255_16x4);

             c1_val3_16x8 = vcombine_u16(c1_val3_16x4, temp_16x4);

             c1_val3_8x8 = vshrn_n_u16(c1_val3_16x8, 8);
             res_32x2 = vreinterpret_u32_u8(c1_val3_8x8);
#    else //COLSAME
             c1_val1_16x8 = vcombine_u16(c1_16x4, val1_16x4);
             c2_val3_16x8 = vcombine_u16(c2_16x4, val3_16x4);

             cv_16x4 = vdup_n_u16(cv>>16);
             cv += cd;
             cv_rv_16x8 = vcombine_u16(cv_16x4, rv_16x4);

             c2_val3_16x8 = vsubq_u16(c2_val3_16x8, c1_val1_16x8);
             c2_val3_16x8 = vmulq_u16(c2_val3_16x8, cv_rv_16x8);
             c2_val3_16x8 = vshrq_n_u16(c2_val3_16x8, 8);
             c2_val3_16x8 = vaddq_u16(c2_val3_16x8, c1_val1_16x8);
             c2_val3_16x8 = vandq_u16(c2_val3_16x8, x255_16x8);

             c2_local_16x4 = vget_low_u16(c2_val3_16x8);
             val3_16x4 = vget_high_u16(c2_val3_16x8);

             val3_16x4 = vmul_u16(c2_local_16x4, val3_16x4);
             val3_16x4 = vadd_u16(val3_16x4, x255_16x4);

             val3_16x8 = vcombine_u16(val3_16x4, temp_16x4);

             val3_8x8 = vshrn_n_u16(val3_16x8, 8);
             res_32x2 = vreinterpret_u32_u8(val3_8x8);
#    endif //COLSAME
#   else //COLMUL
             val3_16x4 = vsub_u16(val3_16x4, val1_16x4);
             val3_16x4 = vmul_u16(val3_16x4, rv_16x4);
             val3_16x4 = vshr_n_u16(val3_16x4, 8);
             val3_16x4 = vadd_u16(val3_16x4, val1_16x4);

             val3_16x8 = vcombine_u16(val3_16x4, temp_16x4);

             val3_8x8 = vmovn_u16(val3_16x8);
             res_32x2 = vreinterpret_u32_u8(val3_8x8);
#   endif //COLMUL
             vst1_lane_u32(d, res_32x2, 0);
=======
            FPU_NEON;
#   ifdef COLMUL
            // initialize alpha for interpolation of c1 and c2
            VDUP_NEON(d15, cv >> 16);
            // copy c1 and c2 as algorithm will overwrite it
            VMOV_R2R_NEON(q6, q9);
            cv += cd; // col
#   endif //COLMUL
            VMOV_M2R_NEON(d8, val1);
            VEOR_NEON(q0);
            VMOV_M2R_NEON(d9, val3);
            VMOV_M2R_NEON(d10, val2);
            VEOR_NEON(q1);
            VMOV_M2R_NEON(d11, val4);
            VDUP_NEON(q3, ru);
            VDUP_NEON(d14, rv);
            VZIP_NEON(q4, q0);
            VZIP_NEON(q5, q1);
            VMOV_R2R_NEON(d9, d0);
            VMOV_R2R_NEON(d11, d2);
            // by this point we have all required data in right registers
            // interpolate val1,val2 and val3,val4
            INTERP_256_NEON(q3, q5, q4, q2);
#   ifdef COLMUL
#    ifdef COLSAME
            INTERP_256_NEON(d14, d9, d8, d4);
#    else //COLSAME
            /* move result of val3,val4 interpolation (and c1 if COLMUL is
               defined) for next step */
            VSWP_NEON(d9, d12);
            /* second stage of interpolation, also here c1 and c2 are
               interpolated */
            INTERP_256_NEON(q7, q6, q4, q2);
#    endif //COLSAME
#   else //COLMUL
            INTERP_256_NEON(d14, d9, d8, d4);
#   endif //COLMUL
#   ifdef COLMUL
#    ifdef COLSAME
            MUL4_SYM_NEON(d8, d12, d4);
#    else //COLSAME
            MUL4_SYM_NEON(d8, d9, d4); // do required multiplication
#    endif //COLSAME
#   endif //COLMUL
            VMOV_R2M_NEON(q4, d8, d); // save result to d
>>>>>>> 1c816daeed970349f31eb94763657ae1637dbfda
          }
        else
          *d = val1;
#  else //COLMUL
        val1 = INTERP_256(ru, val2, val1);
        val3 = INTERP_256(ru, val4, val3);
        val1 = INTERP_256(rv, val3, val1); // col
#   ifdef COLMUL
#    ifdef COLSAME
<<<<<<< HEAD
        *d = MUL4_SYM(c1, val1);
#    else //COLSAME
        val2 = INTERP_256((cv >> 16), c2, c1); // col
        *d  = MUL4_SYM(val2, val1); // col
        cv += cd; // col
#    endif //COLSAME
#   else
        *d = val1;
#   endif //COLMUL
=======
        val1 = MUL4_SYM(c1, val1);
#    else //COLSAME
        val2 = INTERP_256((cv >> 16), c2, c1); // col
        val1  = MUL4_SYM(val2, val1); // col
        cv += cd; // col
#    endif //COLSAME
#   endif //COLMUL

>>>>>>> 1c816daeed970349f31eb94763657ae1637dbfda
#  endif //SCALE_USING_MMX
        u += ud;
        v += vd;
# endif //COLBLACK
<<<<<<< HEAD
        if (anti_alias) *d = _aa_coverage_apply(line, ww, w, *d);
=======
        if (anti_alias) val1 = _aa_coverage_apply(line, ww, w, val1);
        *d = val1;
>>>>>>> 1c816daeed970349f31eb94763657ae1637dbfda
        d++;
        ww--;
     }
}
#else //SMOOTH
{
# ifdef SCALE_USING_NEON
#  ifndef COLBLACK
#   ifdef COLMUL
   uint16x4_t x255_16x4;
   uint16x4_t temp_16x4;
   uint16x8_t cval_16x8;
   uint32x2_t res_32x2;
   uint8x8_t cval_8x8;
   uint16x4_t c1_16x4;
   uint16x4_t cval_16x4;
   uint16x4_t val1_16x4;
   uint32x2_t val1_32x2;
   uint8x8_t val1_8x8;

   x255_16x4 = vdup_n_u16(0xff);
#    ifdef COLSAME
<<<<<<< HEAD
   uint16x8_t c1_16x8;
   uint16x8_t val1_16x8;
   uint32x2_t c1_32x2;
   uint8x8_t c1_8x8;

   c1_32x2 = vset_lane_u32(c1, c1_32x2, 0);

   c1_8x8 = vreinterpret_u8_u32(c1_32x2);
   c1_16x8 = vmovl_u8(c1_8x8);

   c1_16x4 = vget_low_u16(c1_16x8);
#    else //COLSAME
   uint16x4_t c2_16x4;
   uint16x4_t c2_c1_16x4;
   uint16x4_t c2_c1_local_16x4;
   uint16x4_t cv_16x4;
   uint16x8_t c1_c2_16x8;
   uint16x8_t val1_16x8;
   uint32x2_t c1_c2_32x2;
   uint8x8_t c1_c2_8x8;

   c1_c2_32x2 = vset_lane_u32(c1, c1_c2_32x2, 0);
   c1_c2_32x2 = vset_lane_u32(c2, c1_c2_32x2, 1);

   c1_c2_8x8 = vreinterpret_u8_u32(c1_c2_32x2);
   c1_c2_16x8 = vmovl_u8(c1_c2_8x8);

   c1_16x4 = vget_low_u16(c1_c2_16x8);
   c2_16x4 = vget_high_u16(c1_c2_16x8);

   c2_c1_16x4 = vsub_u16(c2_16x4, c1_16x4);
#    endif //COLSAME
#   endif //COLMUL
#  endif //COLBLACK
=======
   FPU_NEON;
   VMOV_I2R_NEON(q2, #255);
   VMOV_M2R_NEON(d10, c1);
   VEOR_NEON(d0);
   VZIP_NEON(d10, d0);
#    else
   // c1 and c2 are constants inside the cycle
   FPU_NEON;
   VMOV_I2R_NEON(q2, #255);
   VMOV_M2R_NEON(d10, c1);
   VEOR_NEON(q0);
   VMOV_M2R_NEON(d11, c2);
   VZIP_NEON(q5, q0);
   VMOV_R2R_NEON(d11, d0);
#    endif //COLSAME
#   endif //COLBLACK
#  endif //COLMUL
>>>>>>> 1c816daeed970349f31eb94763657ae1637dbfda
# endif //SCALE_USING_NEON

   while (ww > 0)
     {
<<<<<<< HEAD
# ifndef SCALE_USING_NEON
#  ifdef COLMUL
#   ifndef COLBLACK
        DATA32 val1;
#    ifndef COLSAME
        DATA32 cval; // col
#    endif //COLSAME
#   endif //COLBLACK
#  endif //COLMUL
# endif //SCALE_USING_NEON

# ifdef COLBLACK
        *d = 0xff000000; // col
=======
        DATA32 val1 = 0x00000000;
# ifdef COLMUL
#  ifndef COLBLACK
#   ifdef COLSAME
#   else
        DATA32 cval; // col
#   endif //COLSAME
#  endif  //COLBLACK
# endif //COLMUL

# ifdef COLBLACK
        val1 = 0xff000000; // col
>>>>>>> 1c816daeed970349f31eb94763657ae1637dbfda
# else //COLBLACK
        s = sp + ((v >> (FP + FPI)) * sw) + (u >> (FP + FPI));
#  ifdef COLMUL
#   ifdef SCALE_USING_NEON
#    ifdef COLSAME
        val1_32x2 = vset_lane_u32(*s, val1_32x2, 0);
        val1_8x8 = vreinterpret_u8_u32(val1_32x2);
        val1_16x8 = vmovl_u8(val1_8x8);
        val1_16x4 = vget_low_u16(val1_16x8);
        cval_16x4 = c1_16x4;
#    else //COLSAME
        cv_16x4 = vdup_n_u16(cv>>16);
        cv += cd; // col

        c2_c1_local_16x4 = vmul_u16(c2_c1_16x4, cv_16x4);
        c2_c1_local_16x4 = vshr_n_u16(c2_c1_local_16x4, 8);
        c2_c1_local_16x4 = vadd_u16(c2_c1_local_16x4, c1_16x4);
        cval_16x4 = vand_u16(c2_c1_local_16x4, x255_16x4);
        val1_32x2 = vset_lane_u32(*s, val1_32x2, 0);
        val1_8x8 = vreinterpret_u8_u32(val1_32x2);
        val1_16x8 = vmovl_u8(val1_8x8);
        val1_16x4 = vget_low_u16(val1_16x8);
#    endif //COLSAME
        cval_16x4 = vmul_u16(cval_16x4, val1_16x4);
        cval_16x4 = vadd_u16(cval_16x4, x255_16x4);

        cval_16x8 = vcombine_u16(cval_16x4, temp_16x4);

        cval_8x8 = vshrn_n_u16(cval_16x8, 8);
        res_32x2 = vreinterpret_u32_u8(cval_8x8);

        vst1_lane_u32(d, res_32x2, 0);
#   else //SCALE_USING_NEON
        val1 = *s; // col
<<<<<<< HEAD
#    ifdef COLSAME
        *d = MUL4_SYM(c1, val1);
=======
#   ifdef COLSAME
#    ifdef SCALE_USING_NEON
        VMOV_M2R_NEON(d1, val1);
        VEOR_NEON(d0);
        VZIP_NEON(d1, d0);
        VMOV_R2R_NEON(d0, d10);
        MUL4_SYM_NEON(d0, d1, d4)
        VMOV_R2M_NEON(q0, d0, d);
#    else
        val1 = MUL4_SYM(c1, val1);
#    endif  //SCALE_USING_NEON
#   else //COLSAME
/* XXX: this neon is broken! :( FIXME
#    ifdef SCALE_USING_NEON
        FPU_NEON;
        VMOV_M2R_NEON(d12, val1);
        VMOV_R2R_NEON(q4, q5);
        VEOR_NEON(q1);
        VDUP_NEON(d15, cv >> 16);
        VZIP_NEON(q6, q1);
        INTERP_256_NEON(d15, d9, d8, d4); // interpolate c1 and c2
        MUL4_SYM_NEON(d8, d12, d4); // multiply
        VMOV_R2M_NEON(q4, d8, d); // save result
>>>>>>> 1c816daeed970349f31eb94763657ae1637dbfda
#    else
        cval = INTERP_256((cv >> 16), c2, c1); // col
<<<<<<< HEAD
        *d = MUL4_SYM(cval, val1);
        cv += cd; // col
#    endif
        if (anti_alias) *d = _aa_coverage_apply(line, ww, w, *d);
#   endif
#  else
        *d = *s;
=======
        val1 = MUL4_SYM(cval, val1);
        cv += cd; // col              
/*
#    endif
 */
#   endif //COLSAME
#  else //COLMUL
        val1 = *s;
>>>>>>> 1c816daeed970349f31eb94763657ae1637dbfda
#  endif //COLMUL
        u += ud;
        v += vd;
# endif //COLBLACK
<<<<<<< HEAD
=======
        if (anti_alias) val1 = _aa_coverage_apply(line, ww, w, val1);
        *d = val1;
>>>>>>> 1c816daeed970349f31eb94763657ae1637dbfda
        d++;
        ww--;
     }
}
#endif //SMOOTH
