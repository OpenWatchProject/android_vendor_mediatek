# Copyright Statement:
#
# This software/firmware and related documentation ("MediaTek Software") are
# protected under relevant copyright laws. The information contained herein
# is confidential and proprietary to MediaTek Inc. and/or its licensors.
# Without the prior written permission of MediaTek inc. and/or its licensors,
# any reproduction, modification, use or disclosure of MediaTek Software,
# and information contained herein, in whole or in part, shall be strictly prohibited.
#
# MediaTek Inc. (C) 2019. All rights reserved.
#
# BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
# THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
# RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
# AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
# NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
# SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
# SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
# THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
# THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
# CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
# SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
# STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
# CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
# AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
# OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
# MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
#
# The following software/firmware and/or related documentation ("MediaTek Software")
# have been modified by MediaTek Inc. All revisions are subject to any receiver's
# applicable license agreements with MediaTek Inc.

# LSTM Test, With Cifg, With Peephole, No Projection, No Clipping.

model = Model()

n_batch = 1
n_input = 2
# n_cell and n_output have the same size when there is no projection.
n_cell = 4
n_output = 4

input = Input("input", "TENSOR_FLOAT32", "{%d, %d}" % (n_batch, n_input))

input_to_input_weights = Parameter("input_to_input_weights", "TENSOR_FLOAT32", "{0}", [] )
input_to_forget_weights = Parameter("input_to_forget_weights", "TENSOR_FLOAT32", "{%d, %d}" % (n_cell, n_input), [-0.55291498, -0.42866567, 0.13056988, -0.3633365, -0.22755712, 0.28253698, 0.24407166, 0.33826375])
input_to_cell_weights = Parameter("input_to_cell_weights", "TENSOR_FLOAT32", "{%d, %d}" % (n_cell, n_input), [-0.49770179, -0.27711356, -0.09624726, 0.05100781, 0.04717243, 0.48944736, -0.38535351, -0.17212132])
input_to_output_weights = Parameter("input_to_output_weights", "TENSOR_FLOAT32", "{%d, %d}" % (n_cell, n_input), [0.10725588, -0.02335852, -0.55932593, -0.09426838, -0.44257352, 0.54939759, 0.01533556, 0.42751634])

recurrent_to_input_weights = Parameter("recurrent_to_intput_weights", "TENSOR_FLOAT32", "{0}", [])
recurrent_to_forget_weights = Parameter("recurrent_to_forget_weights", "TENSOR_FLOAT32", "{%d, %d}" % (n_cell, n_output), [
              -0.13832897, -0.0515101, -0.2359007, -0.16661474,
              -0.14340827, 0.36986142, 0.23414481, 0.55899,
              0.10798943, -0.41174671, 0.17751795, -0.34484994,
              -0.35874045, -0.11352962, 0.27268326, 0.54058349])
recurrent_to_cell_weights = Parameter("recurrent_to_cell_weights", "TENSOR_FLOAT32", "{%d, %d}" % (n_cell, n_output), [
              0.54066205, -0.32668582, -0.43562764, -0.56094903,
              0.42957711, 0.01841056, -0.32764608, -0.33027974,
              -0.10826075, 0.20675004, 0.19069612, -0.03026325,
              -0.54532051, 0.33003211, 0.44901288, 0.21193194])
recurrent_to_output_weights = Parameter("recurrent_to_output_weights", "TENSOR_FLOAT32", "{%d, %d}" % (n_cell, n_output), [
              0.41613156, 0.42610586, -0.16495961, -0.5663873,
              0.30579174, -0.05115908, -0.33941799, 0.23364776,
              0.11178309, 0.09481031, -0.26424935, 0.46261835,
              0.50248802, 0.26114327, -0.43736315, 0.33149987])

cell_to_input_weights = Parameter("cell_to_input_weights", "TENSOR_FLOAT32", "{0}", [])
cell_to_forget_weights = Parameter("cell_to_forget_weights", "TENSOR_FLOAT32", "{%d}" % (n_cell), [0.47485286, -0.51955009, -0.24458408, 0.31544167])
cell_to_output_weights = Parameter("cell_to_output_weights", "TENSOR_FLOAT32", "{%d}" % (n_cell), [-0.17135078, 0.82760304, 0.85573703, -0.77109635])

input_gate_bias = Parameter("input_gate_bias", "TENSOR_FLOAT32", "{0}", [])
forget_gate_bias = Parameter("forget_gate_bias", "TENSOR_FLOAT32", "{%d}"%(n_cell), [1.,1.,1.,1.])
cell_gate_bias = Parameter("cell_gate_bias", "TENSOR_FLOAT32", "{%d}"%(n_cell), [0.,0.,0.,0.])
output_gate_bias = Parameter("output_gate_bias", "TENSOR_FLOAT32", "{%d}"%(n_cell), [0.,0.,0.,0.])

projection_weights = Parameter("projection_weights", "TENSOR_FLOAT32", "{0}", [])
projection_bias = Parameter("projection_bias", "TENSOR_FLOAT32", "{0}", [])

output_state_in = Input("output_state_in", "TENSOR_FLOAT32", "{%d, %d}" % (n_batch, n_output))
cell_state_in = Input("cell_state_in", "TENSOR_FLOAT32", "{%d, %d}" % (n_batch, n_cell))

activation_param = Int32Scalar("activation_param", 4)  # Tanh
cell_clip_param = Float32Scalar("cell_clip_param", 0.)
proj_clip_param = Float32Scalar("proj_clip_param", 0.)

scratch_buffer = IgnoredOutput("scratch_buffer", "TENSOR_FLOAT32", "{%d, %d}" % (n_batch, n_cell * 3))
output_state_out = Output("output_state_out", "TENSOR_FLOAT32", "{%d, %d}" % (n_batch, n_output))
cell_state_out = Output("cell_state_out", "TENSOR_FLOAT32", "{%d, %d}" % (n_batch, n_cell))
output = Output("output", "TENSOR_FLOAT32", "{%d, %d}" % (n_batch, n_output))

model = model.Operation("LSTM",
                        input,

                        input_to_input_weights,
                        input_to_forget_weights,
                        input_to_cell_weights,
                        input_to_output_weights,

                        recurrent_to_input_weights,
                        recurrent_to_forget_weights,
                        recurrent_to_cell_weights,
                        recurrent_to_output_weights,

                        cell_to_input_weights,
                        cell_to_forget_weights,
                        cell_to_output_weights,

                        input_gate_bias,
                        forget_gate_bias,
                        cell_gate_bias,
                        output_gate_bias,

                        projection_weights,
                        projection_bias,

                        output_state_in,
                        cell_state_in,

                        activation_param,
                        cell_clip_param,
                        proj_clip_param
).To([scratch_buffer, output_state_out, cell_state_out, output])

input0 = {
    input:           [3., 4.],
    output_state_in: [-0.364445, -0.00352185, 0.128866, -0.0516365],
    cell_state_in:   [-0.760444, -0.0180416, 0.182264, -0.0649371],
}

output0 = {
    scratch_buffer: [ 0 for x in range(n_batch * n_cell * 3) ],
    cell_state_out: [ -0.978419, -0.139203, 0.338163, -0.0983904 ],
    output_state_out: [ -0.423122, -0.0121822, 0.24201, -0.0812458 ],
}
output0[output] = [-0.42312205, -0.01218222, 0.24201041, -0.08124574]
Example((input0, output0))
