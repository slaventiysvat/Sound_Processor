#include "rls.hpp"

RLSState rls_init(int order, double fr_factor, double delta_param) {
    RLSState st;
    st.p = order;
    st.lambda = fr_factor;
    st.lam_inv = 1.0 / fr_factor;
    st.delta = delta_param;
    st.w = Eigen::VectorXd::Zero(order);
    st.P = delta_param * Eigen::MatrixXd::Identity(order, order);
    st.buff_n = Eigen::VectorXd::Zero(order);
    st.buff_x = Eigen::VectorXd::Zero(order);
    return st;
}

Eigen::VectorXd RLSFilterIt(const Eigen::VectorXd &n,
                            const Eigen::VectorXd &x,
                            RLSState &st,
                            int vad_dec) {
    const int p = st.p;
    const double lambda = st.lambda;
    const double lam_inv = st.lam_inv;

    Eigen::VectorXd w = st.w;
    Eigen::MatrixXd P = st.P;

    Eigen::VectorXd x_buf(st.buff_x.size() + x.size());
    x_buf << st.buff_x, x;
    Eigen::VectorXd n_buf(st.buff_n.size() + n.size());
    n_buf << st.buff_n, n;

    st.buff_x = x_buf.tail(p - 1);
    st.buff_n = n_buf.tail(p - 1);

    Eigen::VectorXd e = Eigen::VectorXd::Zero(x_buf.size());
    for (int m = p - 1; m < x_buf.size(); ++m) {
        Eigen::VectorXd y = n_buf.segment(m - (p - 1), p).reverse();
        double err = x_buf(m) - w.dot(y);
        e(m) = err;
        if (vad_dec == 0) {
            Eigen::VectorXd Pi = P * y;
            double denom = lambda + y.dot(Pi);
            Eigen::VectorXd k = Pi / denom;
            P = (P - k * y.transpose() * P) * lam_inv;
            w += k * err;
        }
    }

    st.w = w;
    st.P = P;
    return e.tail(x.size());
}