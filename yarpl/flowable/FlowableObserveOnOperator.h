#pragma once

namespace yarpl {
namespace flowable {
namespace detail {

template <typename T>
class ObserveOnOperatorSubscriber;

template <typename T>
class ObserveOnOperatorSubscription : public yarpl::flowable::Subscription,
                                      public yarpl::enable_get_ref {
 public:
  ObserveOnOperatorSubscription(
      std::shared_ptr<ObserveOnOperatorSubscriber<T>> subscriber,
      std::shared_ptr<Subscription> subscription)
      : subscriber_(std::move(subscriber)),
        subscription_(std::move(subscription)) {}

  // all requesting methods are called from 'executor_' in the
  // associated subscriber
  void cancel() override {
    auto self = this->ref_from_this(this);

    if (auto subscriber = std::move(subscriber_)) {
      subscriber->isCanceled_ = true;
    }

    subscription_->cancel();
  }

  void request(int64_t n) override {
    subscription_->request(n);
  }

 private:
  std::shared_ptr<ObserveOnOperatorSubscriber<T>> subscriber_;
  std::shared_ptr<Subscription> subscription_;
};

template <typename T>
class ObserveOnOperatorSubscriber : public yarpl::flowable::Subscriber<T>,
                                    public yarpl::enable_get_ref {
 public:
  ObserveOnOperatorSubscriber(
      std::shared_ptr<Subscriber<T>> inner,
      folly::Executor& executor)
      : inner_(std::move(inner)), executor_(executor) {}

  // all signaling methods are called from upstream EB
  void onSubscribe(std::shared_ptr<Subscription> subscription) override {
    executor_.add([
      self = this->ref_from_this(this),
      s = std::move(subscription)
    ]() mutable {
      auto subscription =
          std::make_shared<ObserveOnOperatorSubscription<T>>(self, std::move(s));
      self->inner_->onSubscribe(std::move(subscription));
    });
  }
  void onNext(T next) override {
    executor_.add(
        [ self = this->ref_from_this(this), n = std::move(next) ]() mutable {
          if (!self->isCanceled_) {
            self->inner_->onNext(std::move(n));
          }
        });
  }
  void onComplete() override {
    executor_.add([self = this->ref_from_this(this)]() mutable {
      if (!self->isCanceled_) {
        self->inner_->onComplete();
      }
    });
  }
  void onError(folly::exception_wrapper err) override {
    executor_.add(
        [ self = this->ref_from_this(this), e = std::move(err) ]() mutable {
          if (!self->isCanceled_) {
            self->inner_->onError(std::move(e));
          }
        });
  }

 private:
  friend class ObserveOnOperatorSubscription<T>;
  bool isCanceled_{false}; // only accessed in executor_ thread

  std::shared_ptr<Subscriber<T>> inner_;
  folly::Executor& executor_;
};

template <typename T>
class ObserveOnOperator : public yarpl::flowable::Flowable<T> {
 public:
  ObserveOnOperator(std::shared_ptr<Flowable<T>> upstream, folly::Executor& executor)
      : upstream_(std::move(upstream)), executor_(executor) {}

  void subscribe(std::shared_ptr<Subscriber<T>> subscriber) override {
    upstream_->subscribe(std::make_shared<ObserveOnOperatorSubscriber<T>>(
        std::move(subscriber), executor_));
  }

  std::shared_ptr<Flowable<T>> upstream_;
  folly::Executor& executor_;
};
}
}
} /* namespace yarpl::flowable::detail */