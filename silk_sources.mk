SILK_SOURCES = \
silk/silk_CNG.c \
silk/silk_code_signs.c \
silk/silk_create_init_destroy.c \
silk/silk_decode_core.c \
silk/silk_decode_frame.c \
silk/silk_decode_parameters.c \
silk/silk_decode_indices.c \
silk/silk_decode_pulses.c \
silk/silk_decoder_set_fs.c \
silk/silk_dec_API.c \
silk/silk_enc_API.c \
silk/silk_encode_indices.c \
silk/silk_encode_pulses.c \
silk/silk_gain_quant.c \
silk/silk_interpolate.c \
silk/silk_LP_variable_cutoff.c \
silk/silk_NLSF_decode.c \
silk/silk_NSQ.c \
silk/silk_NSQ_del_dec.c \
silk/silk_PLC.c \
silk/silk_shell_coder.c \
silk/silk_tables_gain.c \
silk/silk_tables_LTP.c \
silk/silk_tables_NLSF_CB_NB_MB.c \
silk/silk_tables_NLSF_CB_WB.c \
silk/silk_tables_other.c \
silk/silk_tables_pitch_lag.c \
silk/silk_tables_pulses_per_block.c \
silk/silk_VAD.c \
silk/silk_control_audio_bandwidth.c \
silk/silk_quant_LTP_gains.c \
silk/silk_VQ_WMat_EC.c \
silk/silk_HP_variable_cutoff.c \
silk/silk_NLSF_encode.c \
silk/silk_NLSF_VQ.c \
silk/silk_NLSF_unpack.c \
silk/silk_NLSF_del_dec_quant.c \
silk/silk_process_NLSFs.c \
silk/silk_stereo_LR_to_MS.c \
silk/silk_stereo_MS_to_LR.c \
silk/silk_check_control_input.c \
silk/silk_control_SNR.c \
silk/silk_init_encoder.c \
silk/silk_control_codec.c \
silk/silk_A2NLSF.c \
silk/silk_ana_filt_bank_1.c \
silk/silk_apply_sine_window.c \
silk/silk_array_maxabs.c \
silk/silk_autocorr.c \
silk/silk_biquad_alt.c \
silk/silk_burg_modified.c \
silk/silk_bwexpander_32.c \
silk/silk_bwexpander.c \
silk/silk_debug.c \
silk/silk_decode_pitch.c \
silk/silk_inner_prod_aligned.c \
silk/silk_k2a.c \
silk/silk_k2a_Q16.c \
silk/silk_lin2log.c \
silk/silk_log2lin.c \
silk/silk_LPC_analysis_filter.c \
silk/silk_LPC_inv_pred_gain.c \
silk/silk_table_LSF_cos.c \
silk/silk_NLSF2A.c \
silk/silk_NLSF_stabilize.c \
silk/silk_NLSF_VQ_weights_laroia.c \
silk/silk_pitch_analysis_core.c \
silk/silk_pitch_est_tables.c \
silk/silk_resampler.c \
silk/silk_resampler_down2_3.c \
silk/silk_resampler_down2.c \
silk/silk_resampler_down3.c \
silk/silk_resampler_private_AR2.c \
silk/silk_resampler_private_ARMA4.c \
silk/silk_resampler_private_copy.c \
silk/silk_resampler_private_down4.c \
silk/silk_resampler_private_down_FIR.c \
silk/silk_resampler_private_IIR_FIR.c \
silk/silk_resampler_private_up2_HQ.c \
silk/silk_resampler_private_up4.c \
silk/silk_resampler_rom.c \
silk/silk_resampler_up2.c \
silk/silk_scale_copy_vector16.c \
silk/silk_scale_vector.c \
silk/silk_schur64.c \
silk/silk_schur.c \
silk/silk_sigm_Q15.c \
silk/silk_sort.c \
silk/silk_sum_sqr_shift.c \
silk/silk_stereo_decode_pred.c \
silk/silk_stereo_encode_pred.c \
silk/silk_stereo_find_predictor.c \
silk/silk_stereo_quant_pred.c


if FIXED_POINT
SILK_SOURCES += \
silk/fixed/silk_LTP_analysis_filter_FIX.c \
silk/fixed/silk_LTP_scale_ctrl_FIX.c \
silk/fixed/silk_corrMatrix_FIX.c \
silk/fixed/silk_encode_frame_FIX.c \
silk/fixed/silk_find_LPC_FIX.c \
silk/fixed/silk_find_LTP_FIX.c \
silk/fixed/silk_find_pitch_lags_FIX.c \
silk/fixed/silk_find_pred_coefs_FIX.c \
silk/fixed/silk_noise_shape_analysis_FIX.c \
silk/fixed/silk_prefilter_FIX.c \
silk/fixed/silk_process_gains_FIX.c \
silk/fixed/silk_regularize_correlations_FIX.c \
silk/fixed/silk_residual_energy16_FIX.c \
silk/fixed/silk_residual_energy_FIX.c \
silk/fixed/silk_solve_LS_FIX.c \
silk/fixed/silk_warped_autocorrelation_FIX.c
else
SILK_SOURCES += \
silk/float/silk_apply_sine_window_FLP.c \
silk/float/silk_corrMatrix_FLP.c \
silk/float/silk_encode_frame_FLP.c \
silk/float/silk_find_LPC_FLP.c \
silk/float/silk_find_LTP_FLP.c \
silk/float/silk_find_pitch_lags_FLP.c \
silk/float/silk_find_pred_coefs_FLP.c \
silk/float/silk_LPC_analysis_filter_FLP.c \
silk/float/silk_LTP_analysis_filter_FLP.c \
silk/float/silk_LTP_scale_ctrl_FLP.c \
silk/float/silk_noise_shape_analysis_FLP.c \
silk/float/silk_prefilter_FLP.c \
silk/float/silk_process_gains_FLP.c \
silk/float/silk_regularize_correlations_FLP.c \
silk/float/silk_residual_energy_FLP.c \
silk/float/silk_solve_LS_FLP.c \
silk/float/silk_warped_autocorrelation_FLP.c \
silk/float/silk_wrappers_FLP.c \
silk/float/silk_autocorrelation_FLP.c \
silk/float/silk_burg_modified_FLP.c \
silk/float/silk_bwexpander_FLP.c \
silk/float/silk_energy_FLP.c \
silk/float/silk_inner_product_FLP.c \
silk/float/silk_k2a_FLP.c \
silk/float/silk_levinsondurbin_FLP.c \
silk/float/silk_LPC_inv_pred_gain_FLP.c \
silk/float/silk_pitch_analysis_core_FLP.c \
silk/float/silk_scale_copy_vector_FLP.c \
silk/float/silk_scale_vector_FLP.c \
silk/float/silk_schur_FLP.c \
silk/float/silk_sort_FLP.c
endif
