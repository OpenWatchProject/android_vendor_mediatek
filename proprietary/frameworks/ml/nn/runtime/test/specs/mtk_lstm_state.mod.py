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

# LSTM Test: No Cifg, No Peephole, No Projection, and No Clipping.

model = Model()

n_batch = 1
n_input = 2
# n_cell and n_output have the same size when there is no projection.
n_cell = 4
n_output = 4

input = Input("input", "TENSOR_FLOAT32", "{%d, %d}" % (n_batch, n_input))

input_to_input_weights = Parameter("input_to_input_weights", "TENSOR_FLOAT32", "{%d, %d}" % (n_cell, n_input), [-0.45018822, -0.02338299, -0.0870589, -0.34550029, 0.04266912, -0.15680569, -0.34856534, 0.43890524])
input_to_forget_weights = Parameter("input_to_forget_weights", "TENSOR_FLOAT32", "{%d, %d}" % (n_cell, n_input), [0.09701663, 0.20334584, -0.50592935, -0.31343272, -0.40032279, 0.44781327, 0.01387155, -0.35593212])
input_to_cell_weights = Parameter("input_to_cell_weights", "TENSOR_FLOAT32", "{%d, %d}" % (n_cell, n_input), [-0.50013041, 0.1370284, 0.11810488, 0.2013163, -0.20583314, 0.44344562, 0.22077113, -0.29909778])
input_to_output_weights = Parameter("input_to_output_weights", "TENSOR_FLOAT32", "{%d, %d}" % (n_cell, n_input), [-0.25065863, -0.28290087, 0.04613829, 0.40525138, 0.44272184, 0.03897077, -0.1556896, 0.19487578])

recurrent_to_input_weights = Parameter("recurrent_to_intput_weights", "TENSOR_FLOAT32", "{%d, %d}" % (n_cell, n_output), [
              -0.0063535, -0.2042388, 0.31454784, -0.35746509, 0.28902304, 0.08183324,
            -0.16555229, 0.02286911, -0.13566875, 0.03034258, 0.48091322,
            -0.12528998, 0.24077177, -0.51332325, -0.33502164, 0.10629296])
recurrent_to_forget_weights = Parameter("recurrent_to_forget_weights", "TENSOR_FLOAT32", "{%d, %d}" % (n_cell, n_output), [
              -0.48684245, -0.06655136, 0.42224967, 0.2112639, 0.27654213, 0.20864892,
            -0.07646349, 0.45877004, 0.00141793, -0.14609534, 0.36447752, 0.09196436,
            0.28053468, 0.01560611, -0.20127171, -0.01140004])
recurrent_to_cell_weights = Parameter("recurrent_to_cell_weights", "TENSOR_FLOAT32", "{%d, %d}" % (n_cell, n_output), [
              -0.3407414, 0.24443203, -0.2078532, 0.26320225, 0.05695659, -0.00123841,
            -0.4744786, -0.35869038, -0.06418842, -0.13502428, -0.501764, 0.22830659,
            -0.46367589, 0.26016325, -0.03894562, -0.16368064])
recurrent_to_output_weights = Parameter("recurrent_to_output_weights", "TENSOR_FLOAT32", "{%d, %d}" % (n_cell, n_output), [
              0.43385774, -0.17194885, 0.2718237, 0.09215671, 0.24107647, -0.39835793,
              0.18212086, 0.01301402, 0.48572797, -0.50656658, 0.20047462, -0.20607421,
              -0.51818722, -0.15390486, 0.0468148, 0.39922136])

cell_to_input_weights = Parameter("cell_to_input_weights", "TENSOR_FLOAT32", "{0}", [])
cell_to_forget_weights = Parameter("cell_to_forget_weights", "TENSOR_FLOAT32", "{0}", [])
cell_to_output_weights = Parameter("cell_to_output_weights", "TENSOR_FLOAT32", "{0}", [])

input_gate_bias = Parameter("input_gate_bias", "TENSOR_FLOAT32", "{%d}"%(n_cell), [0.,0.,0.,0.])
forget_gate_bias = Parameter("forget_gate_bias", "TENSOR_FLOAT32", "{%d}"%(n_cell), [1.,1.,1.,1.])
cell_gate_bias = Parameter("cell_gate_bias", "TENSOR_FLOAT32", "{%d}"%(n_cell), [0.,0.,0.,0.])
output_gate_bias = Parameter("output_gate_bias", "TENSOR_FLOAT32", "{%d}"%(n_cell), [0.,0.,0.,0.])

projection_weights = Parameter("projection_weights", "TENSOR_FLOAT32", "{0,0}", [])
projection_bias = Parameter("projection_bias", "TENSOR_FLOAT32", "{0}", [])

output_state_in = Input("output_state_in", "TENSOR_FLOAT32", "{%d, %d}" % (n_batch, n_output))
cell_state_in = Input("cell_state_in", "TENSOR_FLOAT32", "{%d, %d}" % (n_batch, n_cell))

activation_param = Int32Scalar("activation_param", 4)  # Tanh
cell_clip_param = Float32Scalar("cell_clip_param", 0.)
proj_clip_param = Float32Scalar("proj_clip_param", 0.)

scratch_buffer = IgnoredOutput("scratch_buffer", "TENSOR_FLOAT32", "{%d, %d}" % (n_batch, (n_cell * 4)))
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

# Example 1. Input in operand 0,
input0 = {
    input:           [3., 4.],
    output_state_in: [-0.0297319, 0.122947, 0.208851, -0.153588],
    cell_state_in:   [-0.145439, 0.157475, 0.293663, -0.277353]
}
golden_output = [-0.03716109, 0.12507336, 0.41193449, -0.20860538]
output0 = {
    scratch_buffer: [ 0 for x in range(n_batch * n_cell * 4) ],
    cell_state_out: [ -0.287121, 0.148115, 0.556837, -0.388276 ],
    output_state_out: [ -0.0371611, 0.125073, 0.411934, -0.208605 ],
    output: golden_output
}
Example((input0, output0))
